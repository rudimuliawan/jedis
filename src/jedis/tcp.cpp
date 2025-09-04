//
// Created by rudi on 9/3/25.
//

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

#include <jedis/tcp.h>

using Jedis::TcpSocket;

// Public methods

TcpSocket::TcpSocket() = default;

TcpSocket::~TcpSocket() = default;

void TcpSocket::SetupPort(
  const int port
)
{
  port_ = port;
}

void TcpSocket::SetupAddress(
  const std::string& address
)
{
  address_ = address;
}

void TcpSocket::Accept(const std::function<void(int)>& callback) {
  CreateSocketFd_();
  Bind_();
  Listen_();

  while (true) {
    // accept
    sockaddr_in client_address = {};
    socklen_t addr_len = sizeof(client_address);
    const auto conn_fd = accept(fd_, (struct sockaddr *) &client_address, &addr_len);
    if (conn_fd < 0) {
      continue;
    }

    callback(conn_fd);
    close(conn_fd);
  }
}

// Private methods

void TcpSocket::CreateSocketFd_() {
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    throw std::runtime_error("Error : socket()");
  }

  constexpr int val = 1;
  setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
}

void TcpSocket::Bind_() const {
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  addr.sin_addr.s_addr = htonl(0);

  const auto rv = bind(fd_, (const struct sockaddr *) &addr, sizeof(addr));
  if (rv) {
    throw std::runtime_error("Error : bind()");
  }
}

void TcpSocket::Listen_() const {
  const auto rv = listen(fd_, SOMAXCONN);
  if (rv) {
    throw std::runtime_error("Error : listen()");
  }
}
