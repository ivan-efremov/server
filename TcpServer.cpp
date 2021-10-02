#include "TcpServer.h"
#include <iostream>
#include <memory>
#include <utility>

using ba::ip::tcp;

Connection::Connection(tcp::socket socket)
  : m_socket(std::move(socket))
{
  m_socket.set_option(ba::ip::tcp::no_delay(true));
  m_socket.set_option(ba::socket_base::keep_alive(true));
  m_socket.set_option(ba::socket_base::receive_buffer_size(BUFFER_SIZE));
  m_socket.set_option(ba::socket_base::send_buffer_size(BUFFER_SIZE));
}

void Connection::accept()
{
  doRead();
}

void Connection::doRead()
{
  auto self(shared_from_this());
  m_socket.async_read_some(
    ba::buffer(m_buffer),
    [this, self](const bs::error_code& err, std::size_t length) {
      if(!err) {
        //std::cout << std::string(m_buffer.data(), length) << std::endl; sleep(1);
        const char *line =
          "HTTP/1.0 200 OK\r\n"
          "Content-Type: text/html; charset=UTF-8\r\n"
          "Server: Test\r\n"
          "Connection: close\r\n"
          "Content-Length: 0\r\n\r\n";
        length = strlen(line);
        std::memcpy(m_buffer.data(), line, length);
        //doRead();
        doWrite(length);
      } else if(err == ba::error::eof) {
        // client close connection
      } else {
        throw std::runtime_error(std::string("Connection::doRead(): ") + err.message());
      }
    }
  );
}

void Connection::doWrite(std::size_t length)
{
  auto self(shared_from_this());
  ba::async_write(
    m_socket,
    ba::buffer(m_buffer),
    [this, self](const bs::error_code& err, std::size_t transferred) {
      if(err) {
        throw std::runtime_error(std::string("Connection::doWrite(): ") + err.message());
      } else {
        //doRead();
      }
    }
  );
}


TcpServer::TcpServer(ba::io_service& ioService, const std::string& host, uint16_t port)
  : m_acceptor(ioService),
    m_socket(ioService),
    m_host(host),
    m_port(port)
{
}

void TcpServer::start()
{
  ba::ip::tcp::endpoint endpoint(ba::ip::address::from_string(m_host), m_port);
  m_acceptor.open(endpoint.protocol());
  m_acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
  m_acceptor.set_option(ba::ip::tcp::no_delay(true));
  m_acceptor.bind(endpoint);
  m_acceptor.listen(1000);
  doAccept();
}

void TcpServer::stop()
{
  bs::error_code ignore;
  m_acceptor.close(ignore);
  m_socket.shutdown(
    ba::ip::tcp::socket::shutdown_both,
    ignore
  );
  m_socket.close(ignore);
}

void TcpServer::doAccept()
{
  m_acceptor.async_accept(
    m_socket,
    [this](bs::error_code err) {
      if(err) {
        throw std::runtime_error(std::string("TcpServer::doAccept(): ") + err.message());
      } else {
        std::make_shared<Connection>(std::move(m_socket))->accept();
      }
      doAccept();
    }
  );
}
