//
// Created by Rudi Muliawan on 22/03/25.
//
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <jedis-utils.h>
#include <stdio.h>

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        die("socket()");
    }

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = ntohs(8000);
    address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int rv = connect(sock_fd, (const struct sockaddr *) &address, sizeof(address));
    if (rv) {
        die("connect()");
    }

    char message[] = "hello";
    write(sock_fd, &message, strlen(message));

    char read_buffer[64];
    bzero(&read_buffer, sizeof(read_buffer) - 1);

    ssize_t n = read(sock_fd, &read_buffer, sizeof(read_buffer)-1);
    if (n < 0) {
        die("read()");
    }

    printf("server says: %s\n", read_buffer);
    close(sock_fd);
}