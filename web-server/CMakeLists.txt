cmake_minimum_required(VERSION 3.5)

project(web-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(web-server main.cpp server.h server.cpp socket_req.h socket_req.cpp)
