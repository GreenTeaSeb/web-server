#ifndef SOCKET_REQ_H
#define SOCKET_REQ_H
#include <string>
#include <sys/socket.h>
#include <vector>
class socket_req
{

  std::string line = {};

public:
  socket_req(int socketFD);
  int socketFD = {};
  std::string ip = {};
  int port = {};

  bool repsone_ready = false;

  std::string response_header = "HTTP/1.1 ";
  std::vector<uint8_t> response_body = {};
  void read_data();

private:
  void parse_header();
  void parse_line(std::string line);
  std::string url_decode(std::string line);
  std::vector<uint8_t> get_file_data(std::string path);
};

#endif // SOCKET_REQ_H
