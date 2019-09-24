#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

namespace ba = boost::asio;
namespace bs = boost::system;

using boost::asio::ip::tcp;


class Connection:
    public std::enable_shared_from_this<Connection>
{
    enum {
        BUFFER_SIZE = 65535
    };
public:
    Connection(tcp::socket socket)
    : m_socket(std::move(socket))
  {
    const struct timeval tv = { 60, 0 };
    m_socket.set_option(ba::ip::tcp::no_delay(true));
    m_socket.set_option(ba::socket_base::reuse_address(true));
    m_socket.set_option(ba::socket_base::keep_alive(true));
    m_socket.set_option(ba::socket_base::receive_buffer_size(BUFFER_SIZE));
    m_socket.set_option(ba::socket_base::send_buffer_size(BUFFER_SIZE));
    setsockopt(m_socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(m_socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
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
        boost::asio::buffer(m_data, BUFFER_SIZE),
        [this, self](boost::system::error_code err, std::size_t length) {
            if(!err) {
                std::cout << m_data << std::endl;
                doRead();
//                doWrite(length);
            }
        }
    );
  }

  void doWrite(std::size_t length)
  {
    auto self(shared_from_this());
    boost::asio::async_write(m_socket, boost::asio::buffer(m_data, length),
        [this, self](boost::system::error_code err, std::size_t /*length*/) {
            if(!err) {
                doRead();
            }
        }
    );
  }
private:
  tcp::socket m_socket;
  char        m_data[BUFFER_SIZE];
};


class TcpServer
{
public:
  TcpServer(boost::asio::io_service& io_service, short port)
    : m_acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
      m_socket(io_service)
  {
    m_acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
    m_acceptor.set_option(ba::ip::tcp::no_delay(true));
    m_acceptor.set_option(ba::socket_base::receive_buffer_size(65535));
    m_acceptor.set_option(ba::socket_base::send_buffer_size(65535));
    doAccept();
  }
private:
  void doAccept()
  {
    m_acceptor.async_accept(
        m_socket,
        [this](boost::system::error_code err) {
            if(!err) {
                std::make_shared<Connection>(std::move(m_socket))->accept();
            }
            doAccept();
        }
    );
  }
private:
  tcp::acceptor m_acceptor;
  tcp::socket   m_socket;
};


int main(int argc, char* argv[])
{
  try {
    if(argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }
    boost::asio::io_service ioService;
    TcpServer server(ioService, std::atoi(argv[1]));
    while(1) {
        ioService.run_one();
    }
  } catch(const std::exception& err) {
    std::cerr << "Exception: " << err.what() << "\n";
  }
  return 0;
}
