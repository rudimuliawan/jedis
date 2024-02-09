#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>


static void message(const char *message)
{
    fprintf(stderr, "%s\n", message);
}


static void die(const char *message)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, message);
    abort();
}

static void do_something(int conn_fd) {
    char read_buffer[64] = {};
    ssize_t n = read(conn_fd, read_buffer, sizeof(read_buffer) - 1);
    if (n < 0) {
        message("read() error");
        return;
    }
    printf("client says: %s\n", read_buffer);

    char write_buffer[] = "world";
    write(conn_fd, write_buffer, strlen(write_buffer));
}


int main()
{
    // socket()
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind()
    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(8000);
    address.sin_addr.s_addr = ntohl(0);
    int rv = bind(fd, (const struct sockaddr *) &address, sizeof(address));
    if (rv) {
        die("bind()");
    }

    // listen()
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }

    while (true) {
        struct sockaddr_in client_addr;
        bzero(&client_addr, sizeof(client_addr));
        socklen_t socklen = sizeof(client_addr);

        int conn_fd = accept(fd, (struct sockaddr *) &client_addr, &socklen);
        if (conn_fd < 0) {
            continue;
        }

        do_something(conn_fd);
        close(conn_fd);
    }

    return 0;
}
