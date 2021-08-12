#ifndef SERVER_H
#define SERVER_H

#include "socket_req.h"
#include <map>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>

class server
{
  int socket_af = {};
  int epoll_fd = {};
  sockaddr_in server_addr = {};
  std::map<int, socket_req> sockets = {};

  epoll_event events[1024];

public:
  server(std::string ip, int port);
  server(const server&) = delete;
  server& operator=(const server&) = delete;
  ~server();

  void start_listen();
  void accept_incoming();
  void new_client(int fd);

  void add_epoll(int fd, uint32_t events);
  void handle_client(int fd, uint32_t events_sent);

  // SSL
};

#endif // SERVER_H
