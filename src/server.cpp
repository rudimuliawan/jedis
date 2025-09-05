#include <netinet/in.h>
#include <unistd.h>

#include <iostream>

#include <utils.h>

int main() {
    // socket
    const auto fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw std::runtime_error("Error : socket()");
    }

    constexpr int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(0);
    auto rv = bind(fd, (const struct sockaddr *) &addr, sizeof(addr));
    if (rv) {
        throw std::runtime_error("Error : bind()");
    }

    // listen
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        throw std::runtime_error("Error : listen()");
    }

    // accept
    while (true) {
        // accept
        sockaddr_in client_address = {};
        socklen_t addr_len = sizeof(client_address);
        const auto conn_fd = accept(fd, (struct sockaddr *) &client_address, &addr_len);
        if (conn_fd < 0) {
            continue;
        }

        while (true) {
            const int32_t err = one_request(conn_fd);
            if (err) {
                break;
            }
        }

        close(conn_fd);
    }
}
