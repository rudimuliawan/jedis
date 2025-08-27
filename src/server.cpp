#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <utils.h>

void do_something(int conn_fd) {
    char r_buffer[64] = {};
    if (const ssize_t n = read(conn_fd, r_buffer, sizeof(r_buffer) - 1); n < 0) {
        std::cout << "read() error" << std::endl;
        return;
    }

    std::cout << "client says : " << r_buffer << std::endl;

    constexpr char w_buff[] = "world";
    write(conn_fd, w_buff, strlen(w_buff));
}

[[noreturn]] int main() {
    auto fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        DIE("socket()");
    }

    constexpr int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = htonl(0);

    int rv = bind(fd, (const struct sockaddr*) &addr, sizeof(addr));
    if (rv) {
        DIE("bind()");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv) {
        DIE("listen()");
    }

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int conn_fd = accept(fd, (struct sockaddr *) &client_addr, &addrlen);
        if (conn_fd < 0) {
            continue;
        }

        do_something(conn_fd);
        close(conn_fd);
    }
}