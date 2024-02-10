//
// Created by rudi on 2/10/24.
//
#pragma once

#include <cstdlib>

namespace jedis {
    const size_t k_max_msg = 4096;

    int32_t read_full(int, char *, size_t);
    int32_t write_all(int, char *, size_t);
    int32_t query(int, const char *);
}
