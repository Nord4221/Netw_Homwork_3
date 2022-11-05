#ifndef SOCKETCONTROLLER_H
#define SOCKETCONTROLLER_H
#include <cstdlib>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#include "color_input.h"

class SocketController
{
public:
    SocketController();
    ~SocketController();
    int server_socket_init(int server_port);
    int client_socket_init();
    int read_data(char* data_to_send, int data_size);
    int write_data(char* data_to_send, int data_size);
    char*   get_hostname();
    char*   get_server_info();
    const char *get_client_addr();
    int     get_client_port();
private:
//Server Part:
    socket_wrapper::SocketWrapper* _sock_wrap_ptr = nullptr;
    socket_wrapper::Socket* _server_socket_ptr = nullptr;
    char _hostname[NI_MAXHOST];
    char _server_info[NI_MAXSERV];
    int _server_port;
    sockaddr_in _server_addr;\
//Client Part:
    socket_wrapper::Socket* _client_socket_ptr = nullptr;
    struct sockaddr_in _client_address = {0};
    socklen_t _client_address_len = sizeof(sockaddr_in);
    char _client_address_buf[INET_ADDRSTRLEN];
};

#endif // SOCKETCONTROLLER_H
