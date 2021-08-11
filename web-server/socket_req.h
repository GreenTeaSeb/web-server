#ifndef SOCKET_REQ_H
#define SOCKET_REQ_H
#include <string>
#include <sys/socket.h>
#include <vector>
class socket_req
{

  std::string line = {};

public:
  int socketFD = {};
  std::string ip = {};
  int port = {};
  socket_req(int socketFD);

  void read_data();
  void parse_line(std::string l);
};

#endif // SOCKET_REQ_H
