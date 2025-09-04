//
// Created by rudi on 9/4/25.
//

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include <jedis/message.h>

using Jedis::K_MAX_MSG;

int32_t Jedis::read_full(const int fd, char *buffer, size_t n) {
  while (n > 0) {
    ssize_t rv = read(fd, buffer, n);
    if (rv <= 0) {
      return -1;
    }

    assert((size_t) rv <= n);
    n -= (size_t) rv;
    buffer += n;
  }

  return 0;
}

int32_t Jedis::write_all(int fd, char *buffer, std::size_t n) {
  while (n > 0) {
    ssize_t rv = write(fd, buffer, n);
    if (rv <= 0) {
      return -1;
    }

    assert((size_t) rv <= n);
    n -= (size_t) rv;
    buffer += rv;
  }

  return 0;
}

int32_t Jedis::one_request(int conn_fd) {
  char r_buffer[4 + K_MAX_MSG];
  errno = 0;
  int32_t err = read_full(conn_fd, r_buffer, 4);
  if (err) {
    std::cout << (errno == 9 ? "EOF" : "read() error") << std::endl;
    return err;
  }

  uint32_t len = 0;
  memcpy(&len, r_buffer, 4);
  if (len > K_MAX_MSG) {
    std::cout << "too long" << std::endl;
    return -1;
  }

  err = read_full(conn_fd, &r_buffer[4], len);
  if (err) {
    std::cout << "read() error" << std::endl;
    return err;
  }

  std::cout << "client says : " << &r_buffer[4] << std::endl;

  constexpr char reply[] = "world";
  char w_buff[4 + sizeof(reply)];
  len = (uint32_t)strlen(reply);
  memcpy(w_buff, &len, 4);
  memcpy(&w_buff[4], reply, len);
  return write_all(conn_fd, w_buff, 4 + len);
}
