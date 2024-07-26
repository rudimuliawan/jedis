//
// Created by rudi on 7/26/24.
//
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const size_t k_max_msg = 4096;

void die(const char *message)
{
    printf("%s\n", message);
    exit(1);
}

void message(const char *message)
{
    printf("%s\n", message);
}

static int32_t read_full(int fd, char *buffer, size_t n)
{
    while (n > 0) {
        ssize_t read_n = read(fd, buffer, n);
        if (read_n <= 0) {
            return -1;
        }

        assert((size_t) read_n <= n);
        n -= (size_t) read_n;
        buffer += read_n;
    }

    return 0;
}

static int32_t write_all(int fd, char *buffer, size_t n)
{
    while (n > 0) {
        ssize_t write_n = write(fd, buffer, n);
        if (write_n <= 0) {
            return -1;
        }

        assert((size_t) write_n <= n);
        n -= (size_t) write_n;
        buffer += write_n;
    }

    return 0;
}

static int32_t query(int fd, const char *text)
{
    auto len = (uint32_t) strlen(text);
    if (len > k_max_msg) {
        return -1;
    }

    char write_buffer[4 + k_max_msg];
    memcpy(write_buffer, &len, 4);
    memcpy(&write_buffer[4], text, len);
    if (int32_t err = write_all(fd, write_buffer, 4 + len)) {
        return err;
    }

    // 4 bytes header
    char read_buffer[4+k_max_msg+1];
    errno = 0;

    int32_t err = read_full(fd, read_buffer, 4);
    if (err) {
        if (errno == 0) {
            message("error : EOF");
        }
        else {
            message("error : read()");
        }

        return err;
    }

    memcpy(&len, read_buffer, 4);
    if (len > k_max_msg) {
        message("too long");
        return -1;
    }

    err = read_full(fd, &read_buffer[4], len);
    if (err) {
        message("read error");
        return err;
    }

    // print message from server
    read_buffer[4+len] = '\0';
    printf("server says : %s\n", &read_buffer[4]);
    return 0;
}

int main()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        die("error : socket()");
    }

    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(1234);
    address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    if (connect(sock_fd, (const struct sockaddr *) &address, sizeof(address))) {
        die("error : connect()");
    }

    uint32_t err = query(sock_fd, "hello 1");
    if (err) {
        goto L_DONE;
    }

    err = query(sock_fd, "hello 2");
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(sock_fd);
    return 0;
}