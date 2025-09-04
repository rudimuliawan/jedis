//
// Created by rudi on 9/4/25.
//

#pragma once

namespace Jedis {

constexpr size_t K_MAX_MSG = 4096;

int32_t read_full(int fd, char *buffer, std::size_t n);
int32_t write_all(int fd, char *buffer, std::size_t n);
int32_t one_request(int conn_fd);

}
