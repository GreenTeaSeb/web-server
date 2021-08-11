#include "socket_req.h"
#include <iostream>
#include <unistd.h>
socket_req::socket_req(int fd)
{
  socketFD = fd;
}

void
socket_req::read_data()
{
  char buffer[512];
  auto len = recv(socketFD, buffer, sizeof(buffer), MSG_DONTWAIT);
  if (len > 0) {

    line.append(buffer, len);
    parse_line(line);

  } else {
    close(socketFD);
  }
}

void
socket_req::parse_line(std::string l)
{
  if (line.ends_with("\n\n") || line.ends_with("\r\n\r\n")) {
    std::cout << line.substr(0, 5);
  }
}

/*
chars get added
check if string contains \n
split it at \n, erase the 1st part
continue doing so until no more \n or \r\n\r\n \n\n



*/
