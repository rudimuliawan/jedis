#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <utils.h>

static int32_t query(const int fd, char *text) {
    auto len = (uint32_t) strlen(text);
    if (len > K_MAX_MSG) {
        return -1;
    }

    // send request
    char wbuf[4 + K_MAX_MSG];
    memcpy(wbuf, &len, 4); // assume little endian
    memcpy(&wbuf[4], text, len);
    if (const int32_t err = write_all(fd, wbuf, 4 + len)) {
        return err;
    }

    // 4 bytes header
    char rbuf[4 + K_MAX_MSG + 1];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
        std::cout << (errno == 0 ? "EOF" : "read() error") << std::endl;
        return err;
    }
    memcpy(&len, rbuf, 4); // assume little endian
    if (len > K_MAX_MSG) {
        std::cout << "too long" << std::endl;
        return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        std::cout << "read() error" << std::endl;
        return err;
    }

    // do something
    printf("server says: %.*s\n", len, &rbuf[4]);
    return 0;
}

int main() {
    const auto fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(8080);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    if (auto rv = connect(fd, (const struct sockaddr*)&addr, sizeof(addr))) {
        die("connect()");
    }

    int32_t err = query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }
    err = query(fd, "hello2");
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(fd);

    return 0;
}
