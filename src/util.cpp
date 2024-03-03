//
// Created by rudi on 2/10/24.
//

#include <cerrno>
#include <cstdlib>
#include <cstdio>

#include <fcntl.h>

#include <util.h>

void util::message(const char *message) {
    fprintf(stderr, "%s\n", message);
}

void util::die(const char *message) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, message);
    abort();
}

void util::set_fd_to_non_blocking(int fd) {
    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (errno) {
        die("fcntl() error");
        return;
    }

    flags |= O_NONBLOCK;

    errno = 0;
    fcntl(fd, F_SETFL, flags);
    if (errno) {
        die("fcntl() error");
    }
}
