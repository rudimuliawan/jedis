//
// Created by rudi on 2/10/24.
//
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdint>

#include <unistd.h>

#include <message_protocol.h>
#include <util.h>
#include <netinet/in.h>


bool try_flush_buffer(jedis::Conn *conn) {
    ssize_t rv = 0;
    do {
        size_t remain = conn->write_buffer_size - conn->write_buffer_sent;
        rv = write(conn->fd, &conn->write_buffer[conn->write_buffer_sent], remain);
    } while (rv < 0 && errno == EINTR);

    if (rv < 0 && errno == EAGAIN) {
        return false;
    }

    if (rv < 0) {
        util::message("write() error");
        conn->state = jedis::STATE_END;
        return false;
    }

    conn->write_buffer_sent += (size_t)rv;
    assert(conn->write_buffer_sent <= conn->write_buffer_size);

    if (conn->write_buffer_sent == conn->write_buffer_size) {
        // response was fully sent, change state back
        conn->state = jedis::STATE_REQ;
        conn->write_buffer_sent = 0;
        conn->write_buffer_size = 0;
        return false;
    }

    return true;
}

void state_res(jedis::Conn *conn) {
    while (try_flush_buffer(conn)) {}
}

void conn_put(std::vector<jedis::Conn *> &fd2conn, struct jedis::Conn *conn) {
    if (fd2conn.size() <= (size_t) conn->fd) {
        fd2conn.resize(conn->fd + 1);
    }

    fd2conn[conn->fd] = conn;
}

bool try_one_request(jedis::Conn *conn) {
    // try to parse a request from the buffer
    if (conn->read_buffer_size < 4) {
        // not enough data in the buffer. Will retry in the next iteration
        return false;
    }
    uint32_t len = 0;
    memcpy(&len, &conn->read_buffer[0], 4);
    if (len > jedis::k_max_msg) {
        util::message("too long");
        conn->state = jedis::STATE_END;
        return false;
    }
    if (4 + len > conn->read_buffer_size) {
        // not enough data in the buffer. Will retry in the next iteration
        return false;
    }

    // got one request, do something with it
    printf("client says: %.*s\n", len, &conn->read_buffer[4]);

    // generating echoing response
    memcpy(&conn->write_buffer[0], &len, 4);
    memcpy(&conn->write_buffer[4], &conn->read_buffer[4], len);
    conn->write_buffer_size = 4 + len;

    // remove the request from the buffer.
    // note: frequent memmove is inefficient.
    // note: need better handling for production code.
    size_t remain = conn->read_buffer_size - 4 - len;
    if (remain) {
        memmove(conn->read_buffer, &conn->read_buffer[4 + len], remain);
    }
    conn->read_buffer_size = remain;

    // change state
    conn->state = jedis::STATE_RES;
    state_res(conn);

    // continue the outer loop if the request was fully processed
    return (conn->state == jedis::STATE_REQ);
}

bool try_fill_buffer(jedis::Conn *conn) {
    // try to fill the buffer
    assert(conn->read_buffer_size < sizeof(conn->read_buffer));
    ssize_t rv = 0;
    do {
        size_t cap = sizeof(conn->read_buffer) - conn->read_buffer_size;
        rv = read(conn->fd, &conn->read_buffer[conn->read_buffer_size], cap);
    } while (rv < 0 && errno == EINTR);

    if (rv < 0 && errno == EAGAIN) {
        // got EAGAIN, stop.
        return false;
    }

    if (rv < 0) {
        util::message("read() error");
        conn->state = jedis::STATE_END;
        return false;
    }

    if (rv == 0) {
        if (conn->read_buffer_size > 0) {
            util::message("unexpected EOF");
        } else {
            util::message("EOF");
        }
        conn->state = jedis::STATE_END;
        return false;
    }

    conn->read_buffer_size += (size_t)rv;
    assert(conn->read_buffer_size <= sizeof(conn->read_buffer));

    // Try to process requests one by one.
    // Why is there a loop? Please read the explanation of "pipelining".
    while (try_one_request(conn)) {}
    return (conn->state == jedis::STATE_REQ);
}

void state_req(jedis::Conn *conn) {
    while (try_fill_buffer(conn)) {}
}

int32_t jedis::read_full(int fd, char *buffer, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buffer, n);
        if (rv <= 0) {
            return -1;
        }

        assert((ssize_t) rv <= n);
        n -= (ssize_t) rv;
        buffer += rv;
    }

    return 0;
}

int32_t jedis::write_all(int fd, char *buffer, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buffer, n);
        if (rv <= 0) {
            return -1;
        }

        assert((ssize_t) rv <= n);
        n -= (ssize_t) rv;
        buffer += rv;
    }

    return 0;
}

void jedis::connection_io(jedis::Conn *conn) {
    if (conn->state == jedis::STATE_REQ) {
        state_req(conn);
    } else if (conn->state == jedis::STATE_RES) {
        state_res(conn);
    } else {
        assert(0);
    }
}

int32_t jedis::accept_new_conn(std::vector<Conn *> &fd2conn, int fd) {
    // accept
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int conn_fd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
    if (conn_fd < 0) {
        util::message("accept() error");
        return -1;  // error
    }

    // set the new connection fd to nonblocking mode
    util::set_fd_to_non_blocking(conn_fd);

    // creating the struct Conn
    auto *conn = (struct Conn *)malloc(sizeof(struct Conn));
    if (!conn) {
        close(conn_fd);
        return -1;
    }
    conn->fd = conn_fd;
    conn->state = STATE_REQ;
    conn->read_buffer_size = 0;
    conn->write_buffer_size = 0;
    conn->write_buffer_sent = 0;
    conn_put(fd2conn, conn);
    return 0;
}

int32_t jedis::send_req(int fd, const char *text) {
    auto len = (uint32_t)strlen(text);
    if (len > jedis::k_max_msg) {
        return -1;
    }

    char wbuf[4 + jedis::k_max_msg];
    memcpy(wbuf, &len, 4);  // assume little endian
    memcpy(&wbuf[4], text, len);
    return jedis::write_all(fd, wbuf, 4 + len);
}

int32_t jedis::read_res(int fd) {
    // 4 bytes header
    char rbuf[4 + jedis::k_max_msg + 1];
    errno = 0;
    int32_t err = jedis::read_full(fd, rbuf, 4);
    if (err) {
        if (errno == 0) {
            util::message("EOF");
        } else {
            util::message("read() error");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);  // assume little endian
    if (len > jedis::k_max_msg) {
        util::message("too long");
        return -1;
    }

    // reply body
    err = jedis::read_full(fd, &rbuf[4], len);
    if (err) {
        util::message("read() error");
        return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    printf("server says: %s\n", &rbuf[4]);
    return 0;
}