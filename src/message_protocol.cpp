//
// Created by rudi on 2/10/24.
//
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <message_protocol.h>
#include <util.h>

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

int32_t jedis::query(int fd, const char *text) {
    auto len = (uint32_t) strlen(text);
    if (len > jedis::k_max_msg) {
        return -1;
    }

    char write_buffer[4 + jedis::k_max_msg];
    memcpy(&write_buffer, &len, 4);
    memcpy(&write_buffer[4], text, len);
    if (int32_t err = write_all(fd, write_buffer, 4 + len)) {
        return err;
    }

    // 4 bytes header
    char read_buffer[4 + jedis::k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(fd, read_buffer, 4);
    if (err) {
        if (errno == 0) {
            util::message("EOF");
        } else {
            util::message("read() error");
        }

        return err;
    }

    memcpy(&len, read_buffer, 4);
    if (len > jedis::k_max_msg) {
        util::message("too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &read_buffer[4], len);
    if (err) {
        util::message("read() error");
        return err;
    }

    read_buffer[4 + len] = '\0';
    printf("server says: %s\n", &read_buffer[4]);
    return 0;
}
