//
// Created by Rudi Muliawan on 22/03/25.
//

#pragma once

static int32_t read_full(int fd, char *buff, size_t n);
static int32_t write_full(int fd, char *buff, size_t n);

int32_t one_request(int conn_fd);
int32_t query(int conn_fd, const char *text);
