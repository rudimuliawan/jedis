//
// Created by rudi on 11/10/24.
//
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

const size_t k_max_msg = 4096;

void die(char *message) {
    printf("error : %s\n", message);
    exit(-1);
}

static int32_t read_full(int fd, char *buff, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buff, n);
        if (rv <= 0) {
            return -1; // error / unexpected EOF
        }

        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buff += rv;
    }

    return 0;
}

static int32_t write_all(int fd, char *buff, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buff, n);
        if (rv <= 0) {
            return -1; // error
        }

        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buff += rv;
    }

    return 0;
}

static int32_t query(int conn_fd, const char *text) {
    uint32_t len = (uint32_t) strlen(text);
    if (len > k_max_msg) {
        return -1;
    }

    char w_buff[4+k_max_msg];
    memcpy(&w_buff, &len, 4);
    memcpy(&w_buff[4], text, len);
    if (int32_t err = write_all(conn_fd, w_buff, 4+len)) {
        return err;
    }

    // 4 bytes header
    char r_buff[4+k_max_msg];
    errno = 0;
    int32_t err = read_full(conn_fd, r_buff, 4);
    if (err) {
        if (errno == 0) {
            printf("EOF");
        }
        else {
            printf("read() error");
        }
    }

    memcpy(&len, r_buff, 4);
    if (len > k_max_msg) {
        printf("too long");
        return -1;
    }

    // reply body
    err = read_full(conn_fd, &r_buff[4], len);
    if (err) {
        printf("read error");
        return err;
    }

    r_buff[4+len] = '\0';
    printf("server says %s\n", &r_buff[4]);
    return 0;
}

int main(int argc, char *argv[]) {
    // create a socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    // bind the socket
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(8000);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // Wildcard Address 0.0.0.0
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect()");
    }

    // multiple requests
    int32_t err = query(fd, "Hello1");
    if (err) {
        goto L_DONE;
    }

    err = query(fd, "Hello2");
    if (err) {
        goto L_DONE;
    }

    err = query(fd, "Hello3");
    if (err) {
        goto L_DONE;
    }


L_DONE:
    close(fd);
}
