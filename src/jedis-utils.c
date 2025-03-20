//
// Created by rudi on 3/20/25.
//

#include <stdio.h>
#include <stdlib.h>

#include <jedis-utils.h>

void panic(const char *message) {
    printf("%s\n", message);
    exit(1);
}
