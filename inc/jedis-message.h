//
// Created by Rudi Muliawan on 22/03/25.
//

#ifndef JEDIS_MESSAGE_H
#define JEDIS_MESSAGE_H

static int32_t read_full(int fd, char *buff, size_t n);
static int32_t write_full(int fd, char *buff, size_t n);

int32_t one_request(int conn_fd);
int32_t query(int conn_fd, const char *text);

#endif //JEDIS_MESSAGE_H
