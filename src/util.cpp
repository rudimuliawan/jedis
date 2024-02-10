//
// Created by rudi on 2/10/24.
//

#include <cerrno>
#include <cstdlib>
#include <cstdio>

#include <util.h>

void util::message(const char *message) {
    fprintf(stderr, "%s\n", message);
}

void util::die(const char *message) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, message);
    abort();
}