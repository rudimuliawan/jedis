//
// Created by rudi on 2/5/24.
//

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


void die(const char *message)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, message);
    abort();
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

    char message[] = "hello";
    write(fd, message, strlen(message));

    char read_buffer[64] = {};
    ssize_t n = read(fd, read_buffer, sizeof(read_buffer)-1);
    if (n < 0) {
        die("read()");
    }

    printf("server says: %s\n", read_buffer);
    close(fd);

    return 0;
}
