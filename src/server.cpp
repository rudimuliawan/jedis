//
// Created by rudi on 2/5/24.
//

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <netinet/in.h>
#include <unistd.h>

const size_t k_max_msg = 4096;

static void message(const char *message) {
    fprintf(stderr, "%s\n", message);
}

static void die(const char *message) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, message);
    abort();
}

static int32_t read_full(int fd, char *buffer, size_t n) {
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

static int32_t write_all(int fd, char *buffer, size_t n) {
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

static int32_t one_request(int conn_fd) {
    // 4 bytes header
    char read_buffer[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(conn_fd, read_buffer, 4);
    if (err) {
        if (errno == 0) {
            message("EOF");
        } else {
            message("read() error");
        }

        return err;
    }

    uint32_t len = 0;
    memcpy(&len, read_buffer, 4);
    if (len > k_max_msg) {
        message("too long");
        return -1;
    }

    // request body
    err = read_full(conn_fd, &read_buffer[4], len);
    if (err) {
        message("read() error");
        return err;
    }

    read_buffer[4+len] = '\0';
    printf("client says: %s\n", &read_buffer[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char write_buffer[4 + sizeof(reply)];
    len = (uint32_t) strlen(reply);
    memcpy(write_buffer, &len, 4);
    memcpy(&write_buffer[4], reply, len);

    return write_all(conn_fd, write_buffer, 4+len);
}

int main()
{
    // socket()
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind()
    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(8000);
    address.sin_addr.s_addr = ntohl(0);
    int rv = bind(fd, (const struct sockaddr *) &address, sizeof(address));
    if (rv) {
        die("bind()");
    }

    // listen()
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }

    while (true) {
        struct sockaddr_in client_addr = {};
        bzero(&client_addr, sizeof(client_addr));
        socklen_t socklen = sizeof(client_addr);

        int conn_fd = accept(fd, (struct sockaddr *) &client_addr, &socklen);
        if (conn_fd < 0) {
            continue;
        }

        while (true) {
            int32_t err = one_request(conn_fd);
            if (err) {
                break;
            }
        }

        close(fd);
    }

    return 0;
}
