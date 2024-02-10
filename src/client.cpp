//
// Created by rudi on 2/5/24.
//
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <message_protocol.h>
#include <util.h>

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        util::die("socket()");
    }

    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(8000);
    address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
    int rv = connect(fd, (const struct sockaddr *) &address, sizeof(address));
    if (rv) {
        util::die("connect()");
    }

    // multiple requests
    int32_t err = jedis::query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }

    err = jedis::query(fd, "hello2");
    if (err) {
        goto L_DONE;
    }

    err = jedis::query(fd, "hello3");
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(fd);

    return 0;
}
