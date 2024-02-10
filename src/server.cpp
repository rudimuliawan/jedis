//
// Created by rudi on 2/5/24.
//

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <netinet/in.h>
#include <unistd.h>

#include <message_protocol.h>
#include <util.h>

static int32_t one_request(int conn_fd) {
    // 4 bytes inc
    char read_buffer[4 + jedis::k_max_msg + 1];
    errno = 0;
    int32_t err = jedis::read_full(conn_fd, read_buffer, 4);
    if (err) {
        if (errno == 0) {
            util::message("EOF");
        } else {
            util::message("read() error");
        }

        return err;
    }

    uint32_t len = 0;
    memcpy(&len, read_buffer, 4);
    if (len > jedis::k_max_msg) {
        util::message("too long");
        return -1;
    }

    // request body
    err = jedis::read_full(conn_fd, &read_buffer[4], len);
    if (err) {
        util::message("read() error");
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

    return jedis::write_all(conn_fd, write_buffer, 4 + len);
}

int main()
{
    // socket()
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        util::die("socket()");
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
        util::die("bind()");
    }

    // listen()
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        util::die("listen()");
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
