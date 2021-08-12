#include "socket_req.h"
#include <filesystem>
#include <fstream>
#include <unistd.h>
socket_req::socket_req(int fd)
{
  socketFD = fd;
}

void
socket_req::read_data()
{
  char buffer[512];
  auto len = recv(socketFD, buffer, sizeof(buffer), 0);
  if (len > 0) {

    line.append(buffer, len);
    if (line.ends_with("\r\n\r\n") || line.ends_with("\n\n"))
      parse_header();

  } else {
    close(socketFD);
  }
}

void
socket_req::parse_header()
{
  std::string sub = {};
  std::string delim = "\r\n";
  while (line != delim) {
    if (line.ends_with("\r\n")) {
      sub = line.substr(0, line.find("\r\n"));
    } else if (line.ends_with("\n")) {
      sub = line.substr(0, line.find('\n'));
      delim = "\n";
    }
    // parse the line
    parse_line(sub);
    line.erase(0, sub.length() + delim.length());
  }

  response_header.append("Connection: close\nServer: Sveb's server\n");
  response_header.append("\n\n");
  repsone_ready = true;
}

void
socket_req::parse_line(std::string line)
{
  std::string key = line.substr(0, line.find(" "));
  std::string value = line.substr(line.find(" ") + 1, line.length());

  if (key == "GET") {
    std::string path = "." + value.substr(0, value.find(" "));
    printf("requested path: %s\n", path.c_str());
    response_body = get_file_data(path);
  }
}

std::vector<uint8_t>
socket_req::get_file_data(std::string path)
{
  printf("checking path: %s \n", path.c_str());
  if (std::filesystem::exists(path) && path.find("..") == std::string::npos) {

    if (std::filesystem::is_directory(path)) {
      if (std::filesystem::exists(path))
        path.append("/index.html");
      else {
        // todo
        std::string result = {};
        for (auto const& entry : std::filesystem::directory_iterator(path)) {
          result.append(entry.path().generic_string());
          result.push_back('\n');
        }
        return { result.begin(), result.end() };
      }
    }
    std::ifstream file(path, std::ios::in | std::ios::binary);

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    response_header.append("200 OK\n");
    return data;
  } else {
    response_header.append("404\n");
    return {};
  }
}
