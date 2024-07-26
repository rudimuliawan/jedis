#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const size_t k_max_msg = 4096;

void die(const char *message)
{
    printf("%s\n", message);
    exit(1);
}

void message(const char *message)
{
    printf("%s\n", message);
}

static int32_t read_full(int fd, char *buffer, size_t n)
{
    while (n > 0) {
        ssize_t read_n = read(fd, buffer, n);
        if (read_n <= 0) {
            return -1;
        }

        assert((size_t) read_n <= n);
        n -= (size_t) read_n;
        buffer += read_n;
    }

    return 0;
}

static int32_t write_all(int fd, char *buffer, size_t n)
{
    while (n > 0) {
        ssize_t write_n = write(fd, buffer, n);
        if (write_n <= 0) {
            return -1;
        }

        assert((size_t) write_n <= n);
        n -= (size_t) write_n;
        buffer += write_n;
    }

    return 0;
}

int32_t one_request(int conn_fd)
{
    // 4 bytes length
    char read_buffer[4 + k_max_msg + 1];
    errno = 0;

    int32_t err = read_full(conn_fd, read_buffer, 4);
    if (err) {
        if (errno == 0) {
            message("error : EOF");
        }
        else {
            message("error : read()");
        }

        return err;
    }

    uint32_t len = 0;
    memcpy(&len, read_buffer, 4);
    if (len > k_max_msg) {
        message("error : too long");
        return -1;
    }

    // request body
    err = read_full(conn_fd, &read_buffer[4], len);
    if (err) {
        message("error : read()");
        return err;
    }

    // print message
    read_buffer[4 + len] = '\0';
    printf("client says: %s\n", &read_buffer[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char write_buffer[4 + sizeof(reply)];
    len = (uint32_t) strlen(reply);
    memcpy(write_buffer, &len, 4);
    memcpy(&write_buffer[4], reply, len);
    return write_all(conn_fd, write_buffer, 4+len);
}

int main()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        die("error: socket()");
    }

    int val = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = ntohs(1234);
    address.sin_addr.s_addr = ntohl(0);

    if (bind(sock_fd, (const sockaddr *) &address, sizeof(address)) < 0) {
        die("error: bind()");
    }

    if (listen(sock_fd, SOMAXCONN) < 0) {
        die("error: listen()");
    }

    while (true) {
        struct sockaddr_in client_address = {};
        socklen_t socklen = sizeof(client_address);
        int conn_fd = accept(sock_fd, (struct sockaddr *) &client_address, &socklen);
        if (conn_fd < 0) {
            continue;
        }

        while (true) {
            int32_t err = one_request(conn_fd);
            if (err) {
                break;
            }
        }

        close(conn_fd);
    }

    return 0;
}
