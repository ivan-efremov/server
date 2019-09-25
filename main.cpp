#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/array.hpp>


namespace ba = boost::asio;
namespace bs = boost::system;

using boost::asio::ip::tcp;


class Connection:
  public std::enable_shared_from_this<Connection>
{
  enum {
      BUFFER_SIZE = 131072
  };
public:
  Connection(tcp::socket socket)
    : m_socket(std::move(socket))
  {
    m_socket.set_option(ba::ip::tcp::no_delay(true));
    m_socket.set_option(ba::socket_base::keep_alive(true));
//  m_socket.set_option(ba::socket_base::receive_buffer_size(BUFFER_SIZE));
//  m_socket.set_option(ba::socket_base::send_buffer_size(BUFFER_SIZE));
  }
  void accept()
  {
    doRead();
  }
private:
  void doRead()
  {
    auto self(shared_from_this());
    m_socket.async_read_some(
      ba::buffer(m_buffer),
      [this, self](const bs::error_code& err, std::size_t length) {
        if(!err) {
//          std::cout << std::string(m_buffer.data(), length) << std::endl; sleep(1);
          const char *line =
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Server: Test\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n\r\n";
          length = strlen(line);
          std::memcpy(m_buffer.data(), line, length);
//          doRead();
          doWrite(length);
        } else if(err == ba::error::eof) {
          // client close connection
        } else {
          throw std::runtime_error(std::string("Connection::doRead(): ") + err.message());
        }
      }
    );
  }

  void doWrite(std::size_t length)
  {
    auto self(shared_from_this());
    ba::async_write(
      m_socket,
      ba::buffer(m_buffer),
      [this, self](const bs::error_code& err, std::size_t length) {
        if(err) {
          throw std::runtime_error(std::string("Connection::doWrite(): ") + err.message());
        } else {
//          doRead();
        }
      }
    );
  }
private:
  tcp::socket m_socket;
  boost::array<char, BUFFER_SIZE> m_buffer;
};


class TcpServer
{
public:
  TcpServer(ba::io_service& ioService, const std::string& host, uint16_t port)
    : m_acceptor(ioService),
      m_socket(ioService),
      m_host(host),
      m_port(port)
  {
  }
  void start() {
    ba::ip::tcp::endpoint endpoint(ba::ip::address::from_string(m_host), m_port);
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
    m_acceptor.set_option(ba::ip::tcp::no_delay(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen(1000);
    doAccept();
  }
  void stop() {
    bs::error_code ignore;
    m_acceptor.close(ignore);
    m_socket.shutdown(
      ba::ip::tcp::socket::shutdown_both,
      ignore
    );
    m_socket.close(ignore);
  }
private:
  void doAccept()
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
private:
  tcp::acceptor     m_acceptor;
  tcp::socket       m_socket;
  const std::string m_host;
  const uint16_t    m_port;
};


int main(int argc, const char* argv[])
{
  try {
    if(argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }
    ba::io_service ioService;
    TcpServer server(ioService, "0.0.0.0", std::atoi(argv[1]));
    server.start();
    while(1) {
      try {
        ioService.run_one();
      } catch(const std::exception& err) {
        std::cerr << "Error: " << err.what() << "\n";
        ioService.reset();
      }
    }
    server.stop();
  } catch(const std::exception& err) {
    std::cerr << "Error: " << err.what() << "\n";
  }
  return 0;
}
