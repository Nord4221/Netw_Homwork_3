#ifndef SOCKETCONTROLLER_H
#define SOCKETCONTROLLER_H
#include <cstdlib>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#include "color_input.h"


#include <netinet/tcp.h>
#include <sys/ioctl.h>
extern "C"
{
#include <openssl/ssl.h>
#include <openssl/err.h>
}


class SocketController
{
public:
    SocketController();
    ~SocketController();
    int client_socket_init(const char* server_ip, int server_port);
    int init_ssl_wrapper();
    void log_ssl();
    int  read_data(char* data_buffer, int data_size);
    bool write_data(const std::string &request);
    bool get_ssl_rw_error(int msg_length);
    std::string *get_hostname();
private:
//Client Part:
    socket_wrapper::SocketWrapper* _sock_wrap_ptr = nullptr;
    socket_wrapper::Socket* _client_socket_ptr = nullptr;
    int _server_port;
    sockaddr_in _server_addr;\
    std::string* _hostname;
    struct sockaddr_in _server_address = {0};
//SSL Part:
    SSL *_ssl = nullptr;
    SSL_CTX *_ctx = nullptr;
};


#endif // SOCKETCONTROLLER_H
