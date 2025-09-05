//
// Created by Rudi Muliawan on 27/08/25.
//
#pragma once

#include <cstdlib>
#include <string>

constexpr size_t K_MAX_MSG = 4096;

void die(const std::string &message);
int32_t read_full(int fd, char *buffer, size_t n);
int32_t write_all(int fd, char *buffer, std::size_t n);
int32_t one_request(int conn_fd);