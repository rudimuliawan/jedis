//
// Created by rudi on 2/5/24.
//

#include <cstring>
#include <vector>

#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>

#include <message_protocol.h>
#include <util.h>

int main()
{
    // socket()
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        util::die("socket()");
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
        util::die("bind()");
    }

    // listen()
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        util::die("listen()");
    }

    // a map of all client connections
    std::vector<jedis::Conn *> fd2conn;

    // set the listen fd to nonblocking mode
    util::set_fd_to_non_blocking(fd);

    // the event loop
    std::vector<struct pollfd> poll_args;
    while (true) {
        poll_args.clear();

        struct pollfd pfd = {fd, POLLIN, 0};
        poll_args.push_back(pfd);

        for (jedis::Conn *conn: fd2conn) {
            if (!conn) {
                continue;
            }

            struct pollfd pfd = {};
            pfd.fd = conn->fd;
            pfd.events = (conn->state == jedis::STATE_REQ) ? POLLIN : POLLOUT;
            pfd.events = pfd.events | POLLERR;
            poll_args.push_back(pfd);
        }

        int rv = poll(poll_args.data(), (nfds_t) poll_args.size(), 1000);
        if (rv < 0) {
            util::die("poll()");
        }

        for (size_t i = 1; i < poll_args.size(); ++i) {
            if (poll_args[i].revents) {
                jedis::Conn *conn = fd2conn[poll_args[i].fd];
                jedis::connection_io(conn);

                if (conn->state == jedis::STATE_END) {
                    fd2conn[conn->fd] = nullptr;
                    close(conn->fd);
                    free(conn);
                }
            }
        }

        if (poll_args[0].revents) {
            (void) jedis::accept_new_conn(fd2conn, fd);
        }
    }

    return 0;
}



















