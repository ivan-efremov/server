#include <cstdlib>
#include <iostream>
#include "Server.h"

// For test:
//  ab -k -n 100000 -c 10 http://127.0.0.1:8080/
int main(int argc, const char* argv[])
{
  try {
    if(argc != 2) {
      std::cerr << "Usage: server <port>" << std::endl;
      return 1;
    }
    ba::io_service ioService;
    TcpServer server(ioService, "0.0.0.0", std::atoi(argv[1]));
    server.start();
    while(1) {
      try {
        ioService.run_one();
      } catch(const std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        ioService.reset();
      }
    }
    server.stop();
  } catch(const std::exception& err) {
      std::cerr << "Error: " << err.what() << std::endl;
  }
  return 0;
}
