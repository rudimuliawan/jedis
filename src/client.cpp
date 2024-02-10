//
// Created by rudi on 2/5/24.
//

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

const size_t k_max_msg = 4096;

static void message(const char *message) {
    fprintf(stderr, "%s\n", message);
}

void die(const char *message) {
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

static int32_t query(int fd, const char *text) {
    auto len = (uint32_t) strlen(text);
    if (len > k_max_msg) {
        return -1;
    }

    char write_buffer[4 + k_max_msg];
    memcpy(&write_buffer, &len, 4);
    memcpy(&write_buffer[4], text, len);
    if (int32_t err = write_all(fd, write_buffer, 4 + len)) {
        return err;
    }

    // 4 bytes header
    char read_buffer[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(fd, read_buffer, 4);
    if (err) {
        if (errno == 0) {
            message("EOF");
        } else {
            message("read() error");
        }

        return err;
    }

    memcpy(&len, read_buffer, 4);
    if (len > k_max_msg) {
        message("too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &read_buffer[4], len);
    if (err) {
        message("read() error");
        return err;
    }

    read_buffer[4 + len] = '\0';
    printf("server says: %s\n", &read_buffer[4]);
    return 0;
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(8000);
    address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
    int rv = connect(fd, (const struct sockaddr *) &address, sizeof(address));
    if (rv) {
        die("connect()");
    }


    // multiple requests
    int32_t err = query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }

    err = query(fd, "hello2");
    if (err) {
        goto L_DONE;
    }

    err = query(fd, "hello3");
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(fd);

    return 0;
}