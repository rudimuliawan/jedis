//
// Created by Rudi Muliawan on 22/03/25.
//
#include <cerrno>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <jedis-message.hpp>
#include <jedis-utils.hpp>

#define K_MAX_MSG 4096

static int32_t read_full(const int fd, char *buff, size_t n) {
    while (n > 0) {
        const ssize_t rv = read(fd, buff, n);
        if (rv <= 0) {
            return -1;
        }

        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buff += rv;
    }

    return 0;
}

static int32_t write_all(const int fd, char *buff, size_t n) {
    while (n > 0) {
        const ssize_t rv = write(fd, buff, n);
        if (rv <= 0) {
            return -1;
        }

        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buff += rv;
    }

    return 0;
}

int32_t one_request(const int conn_fd) {
    char r_buff[4+K_MAX_MSG];
    errno = 0;

    int32_t err = read_full(conn_fd, r_buff, 4);
    if (err) {
        Utils::msg(errno == 0 ? "EOF" : "read(): error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, r_buff, 4);
    if (len > K_MAX_MSG) {
        Utils::msg("message too long!");
        return -1;
    }

    err = read_full(conn_fd, &r_buff[4], len);
    if (err) {
        Utils::msg(errno == 0 ? "EOF" : "read(): error");
        return err;
    }

    printf("client says: %.*s\n", len, &r_buff[4]);

    const char reply[] = "world";
    char w_buff[4+sizeof(reply)];
    len = (uint32_t) strlen(reply);
    memcpy(w_buff, &len, 4);
    memcpy(&w_buff[4], reply, len);
    return write_all(conn_fd, w_buff, 4+len);
}

int32_t query(const int conn_fd, const char *text) {
    uint32_t len = strlen(text);
    if (len > K_MAX_MSG) {
        return -1;
    }

    char w_buffer[4+K_MAX_MSG];
    memcpy(w_buffer, &len, 4);
    memcpy(&w_buffer[4], text, len);
    int32_t err = write_all(conn_fd, w_buffer, 4+len);
    if (err) {
        return err;
    }

    char r_buffer[4+K_MAX_MSG+1];
    errno = 0;
    err = read_full(conn_fd, r_buffer, 4);
    if (err) {
        Utils::msg(errno == 0 ? "EOF" : "read() error");
        return -1;
    }

    memcpy(&len, r_buffer, 4);
    if (len > K_MAX_MSG) {
        Utils::msg("Too Long");
        return -1;
    }

    err = read_full(conn_fd, &r_buffer[4], len);
    if (err) {
        Utils::msg("read error");
        return err;
    }

    printf("server says: %.*s\n", len, &r_buffer[4]);
    return 0;
}