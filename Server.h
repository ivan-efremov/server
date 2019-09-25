/**
 * @file Server.h
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>


namespace ba = boost::asio;
namespace bs = boost::system;


/**
 * @short class Connection.
 */
class Connection:
  public std::enable_shared_from_this<Connection>
{
  enum {
    BUFFER_SIZE = 131072
  };
public:
  Connection(ba::ip::tcp::socket socket);
  void accept();
private:
  void doRead();
  void doWrite(std::size_t length);
private:
  ba::ip::tcp::socket             m_socket;
  boost::array<char, BUFFER_SIZE> m_buffer;
};


/**
 * @short class TcpServer.
 */
class TcpServer
{
public:
  TcpServer(ba::io_service& ioService, const std::string& host, uint16_t port);
  void start();
  void stop();
private:
  void doAccept();
private:
  ba::ip::tcp::acceptor m_acceptor;
  ba::ip::tcp::socket   m_socket;
  const std::string     m_host;
  const uint16_t        m_port;
};
