#include "socket_req.h"
#include <charconv>
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
    if (line.find("\r\n\r\n") || line.find("\n\n"))
      parse_header();
  }
}

std::string
socket_req::url_decode(std::string line)
{

  for (auto per = line.find("%"); per != -1; per = line.find("%")) {
    if (line.size() - per < 3)
      break;
    uint8_t hex = {};
    std::from_chars(&line[per + 1], &line[per + 3], hex, 16);
    std::string replaced;
    replaced.push_back(hex);
    line.replace(per, 3, replaced);
  }
  return line;
}

void
socket_req::parse_header()
{
  std::string sub = {};
  std::string delim = "\r\n";
  while (line != delim) {
    if (line.find("\r\n")) {
      sub = line.substr(0, line.find("\r\n"));
    } else if (line.ends_with("\n")) {
      sub = line.substr(0, line.find('\n'));
      delim = "\n";
    }
    // parse the lineline.clear();
    sub = url_decode(sub);
    parse_line(sub);
    line.erase(0, sub.length() + delim.length());
  }

  response_header.append("Connection: close\nServer: Sveb's server\n\n");
  repsone_ready = true;
}

void
socket_req::parse_line(std::string line)
{
  if (line.find(" ") > 0 && line.find(" ") < line.size() - 1) {
    std::string key = line.substr(0, line.find(" "));
    std::string value = line.substr(line.find(" ") + 1, line.length());

    if (key == "GET") {
      std::string path = "." + value.substr(0, value.find_last_of(" "));
      // printf("requested path: %s\n", path.c_str());
      response_body = get_file_data(path);
    }
  }
}

std::vector<uint8_t>
socket_req::get_file_data(std::string path)
{
  // printf("checking path: %s \n", path.c_str());
  if (std::filesystem::is_directory(path) &&
      path.find("..") == std::string::npos) {

    return get_file_data(path.append("/index.html"));
  } else {
    if (std::filesystem::exists(path)) {
      std::ifstream file(path, std::ios::in | std::ios::binary);

      std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
      response_header.append("200 OK\n");
      return data;
    }
    response_header.append("404 Not Found\n");
    std::string res = "not found sorry -eh";
    return { res.begin(), res.end() };
  }
}
