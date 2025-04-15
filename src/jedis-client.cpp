//
// Created by Rudi Muliawan on 22/03/25.
//
#include <cstring>

#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <jedis-message.hpp>
#include <jedis-utils.hpp>

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        Utils::die("socket()");
    }

    struct sockaddr_in address {};
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = ntohs(8000);
    address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int rv = connect(sock_fd, (const struct sockaddr *) &address, sizeof(address));
    if (rv) {
        Utils::die("connect()");
    }

    uint32_t err = query(sock_fd, "Hello1");
    if (err) {
        goto L_DONE;;
    }

    query(sock_fd, "Hello2");

L_DONE:
    close(sock_fd);
    return 0;
}