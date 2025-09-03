//
// Created by rudi on 9/3/25.
//
#pragma once

#include <functional>
#include <string>

namespace Jedis {
class TcpSocket {
public:
  /*! \brief Construct a TCP object
   *
   *  \note This constructor doesn't initialize a connection.
   */
  TcpSocket();

  /*! \brief Destructor
   */
  ~TcpSocket();

  /*! \brief SetupPort assign value to port_
   *
   *  \param port  The value that will be assigned to port_.
   */
  void SetupPort(int port);

  /*! \brief SetupPort assign value to address
   *
   *  \param address  The value that will be assigned to address_.
   */
  void SetupAddress(const std::string& address);

  /*! \brief
   */
  void Accept(const std::function<void(int)>& callback);

private:
  int fd_ {};
  int port_ {};
  std::string address_ {};

  void CreateSocketFd_() ;
  void Bind_() const;
  void Listen_() const;
};
}
