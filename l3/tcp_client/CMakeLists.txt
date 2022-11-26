cmake_minimum_required(VERSION 3.10)

project(tcp-client C CXX)

find_package(OpenSSL REQUIRED)

set(${PROJECT_NAME}_SRC color_input.h color_input.cpp socket_controller.cpp socket_controller.h client_msg_analyzer.h client_msg_analyzer.cpp main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" pthread OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries("${PROJECT_NAME}" socket-wrapper)
