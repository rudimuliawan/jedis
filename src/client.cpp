//
// Created by rudi on 2/5/24.
//
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <message_protocol.h>
#include <util.h>

int main() {
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
    const char *query_list[3] = {"hello1", "hello2", "hello3"};
    for (auto &i : query_list) {
        int32_t err = jedis::send_req(fd, i);
        if (err) {
            goto L_DONE;
        }
    }

    for (auto _ : query_list) {
        int32_t err = jedis::read_res(fd);
        if (err) {
            goto L_DONE;
        }
    }

L_DONE:
    close(fd);

    return 0;
}
