//
// Created by rudi on 3/20/25.
//

#include <cstdio>
#include <cstdlib>

#include <jedis-utils.hpp>

void Utils::die(const char *message) {
    printf("error : %s\n", message);
    exit(1);
}

void Utils::msg(const char *message) {
    printf("%s\n", message);
}
