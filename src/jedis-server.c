//
// Created by rudi on 3/20/25.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <jedis-utils.h>

void do_something(int conn_fd) {
    char r_buff[64];
    bzero(&r_buff, sizeof(r_buff));

    const ssize_t n = read(conn_fd, r_buff, sizeof(r_buff)-1);
    if (n < 0) {
        printf("read() error");
        return;
    }

    printf("client says: %s\n", r_buff);

    char w_buff[] = "Worlds";
    write(conn_fd, w_buff, strlen(w_buff));
}

int main() {
    // Create a TCP socket
    const int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        die("socket()");
    }

    // Allow socket to reuse a same address (port) after program restarts
    int val = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(8000);
    address.sin_addr.s_addr = htonl(0);

    int rv = bind(sock_fd, (const struct sockaddr *) &address, sizeof(address));
    if (rv) {
        die("bind()");
    }

    rv = listen(sock_fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }

    while (true) {
        struct sockaddr_in client_address;
        bzero(&client_address, sizeof(client_address));
        socklen_t addr_length = sizeof(client_address);

        const int conn_fd = accept(sock_fd, (struct sockaddr *) &client_address, &addr_length);
        if (conn_fd < 0) {
            continue;
        }

        do_something(conn_fd);

        // do something
        close(conn_fd);
    }
}
