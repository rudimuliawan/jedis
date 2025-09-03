#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <utils.h>

int main() {
    const auto fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        DIE("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(8123);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    if (auto rv = connect(fd, (const struct sockaddr*)&addr, sizeof(addr))) {
        DIE("connect()");
    }

    constexpr char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char r_buff[64] = {};
    if (const ssize_t n = read(fd, r_buff, sizeof(r_buff)-1); n < 0) {
        DIE("read");
    }

    std::cout << r_buff << std::endl;

    return EXIT_SUCCESS;
}