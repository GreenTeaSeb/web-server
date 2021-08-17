#include "server.h"
#include <arpa/inet.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

server::server(std::string ip, int port)
{
  epoll_fd = epoll_create(255);
  if (epoll_fd < 0)
    throw std::runtime_error("failed to create epoll");

  socket_af = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_af == -1)
    throw std::runtime_error("failed to create socket");

  server_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(port),
  };
  server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

  if (bind(socket_af,
           reinterpret_cast<sockaddr*>(&server_addr),
           sizeof(server_addr)) == -1) {
    close(socket_af);
    throw std::runtime_error("failed to bind socket to address");
  }
}

server::~server()
{
  if (socket_af != -1 && socket_af != 0)
    close(socket_af);
  if (epoll_fd > 0)
    close(epoll_fd);
}

void
server::start_listen()
{
  if (listen(socket_af, 10) == -1) {
    close(socket_af);
    throw std::runtime_error("failed to listen");
  }
  add_epoll(socket_af, EPOLLIN | EPOLLPRI);
}
void
server::accept_incoming()
{
  while (true) {
    int epoll_return = epoll_wait(epoll_fd, events, 24, 10);
    if (epoll_return == 0) // timeout
      continue;

    if (epoll_return == -1) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error("epoll_wait error");
    }
    for (auto event = events; event != events + epoll_return; event++) {
      if (event->data.fd == socket_af) { // incoming client request
        new_client(event->data.fd);
        continue;
      }
      handle_client(event->data.fd, event->events);
    }
  }
}

void
server::new_client(int fd)
{
  sockaddr addr = {};
  socklen_t len = sizeof(addr);
  auto client_fd = accept(fd, &addr, &len);
  if (client_fd < 0)
    return;
  add_epoll(client_fd, EPOLLIN | EPOLLPRI);
  auto sock = socket_req{ client_fd };
  sock.port = reinterpret_cast<sockaddr_in*>(&addr)->sin_port;
  sock.ip = inet_ntoa(reinterpret_cast<sockaddr_in*>(&addr)->sin_addr);
  sockets.insert({ client_fd, sock });
  // printf("%d | %s:%d connected\n", sockets.size(), sock.ip.c_str(),
  // sock.port);
}

void
server::add_epoll(int fd, uint32_t events)
{
  epoll_event event = {};
  event.events = events;
  event.data.fd = fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
    throw std::runtime_error("failed to create epoll_ctl");
}

void
server::handle_client(int fd, uint32_t events_sent)
{

  if (sockets.contains(fd)) {
    auto& sock = sockets.at(fd);
    uint32_t err = EPOLLERR | EPOLLHUP;
    if (events_sent & err) {
      close(fd);
      sockets.erase(fd);
      printf("lost %s:%d \n", sock.ip.c_str(), sock.port);
      return;
    }
    sock.read_data();
    if (sock.repsone_ready) {
      send(fd, sock.response_header.data(), sock.response_header.size(), 0);
      if (!sock.response_body.empty())
        send(fd, sock.response_body.data(), sock.response_body.size(), 0);
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    sockets.erase(fd);
  }
}
