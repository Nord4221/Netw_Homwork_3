cmake_minimum_required(VERSION 3.10)

project(tcp_server C CXX)

set(${PROJECT_NAME}_SRC socket_controller.h socket_controller.cpp color_input.h color_input.cpp msg_analyzer.h msg_analyzer.cpp main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" socket-wrapper)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

