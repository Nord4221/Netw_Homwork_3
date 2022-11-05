#include "socket_controller.h"

std::stringstream server_msg_stream;

SocketController::SocketController()
{

}

SocketController::~SocketController()
{
    close(*_server_socket_ptr);
    close(*_client_socket_ptr);
    _sock_wrap_ptr->~SocketWrapper();
}

int SocketController::server_socket_init(int server_port)
{
    _sock_wrap_ptr = new socket_wrapper::SocketWrapper;
    _server_socket_ptr = new socket_wrapper::Socket {AF_INET, SOCK_STREAM, 0};

    server_msg_stream << "Starting echo server on the port " << server_port ;
    print_line(msg_type_ordinary, server_msg_stream);

    if (!(*_server_socket_ptr))
    {
        server_msg_stream << _sock_wrap_ptr->get_last_error_string() << std::endl;
        print_line(msg_type_error, server_msg_stream);
        return EXIT_FAILURE;
    }
    _server_port = server_port;

    _server_addr =
    {
        .sin_family = PF_INET,
        .sin_port = htons(server_port),
    };
    _server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(*_server_socket_ptr, reinterpret_cast<const sockaddr*>(&_server_addr), sizeof(_server_addr)) != 0)
    {
        server_msg_stream << _sock_wrap_ptr->get_last_error_string() << std::endl;
        print_line(msg_type_error, server_msg_stream);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int SocketController::client_socket_init()
{
    listen(*_server_socket_ptr, 1);

    _client_socket_ptr = new socket_wrapper::Socket {AF_INET, SOCK_STREAM, 0};
    *_client_socket_ptr = accept(*_server_socket_ptr, reinterpret_cast<sockaddr *>(&_client_address), &_client_address_len);

    if (!(*_client_socket_ptr))
    {
        server_msg_stream << "Accept failed" << std::endl;
        print_line(msg_type_error, server_msg_stream);
        return EXIT_FAILURE;
    }
    else
    {
        if (getnameinfo(reinterpret_cast<sockaddr *>(&_client_address), _client_address_len,
                        _hostname, NI_MAXHOST, _server_info, NI_MAXSERV, NI_NUMERICSERV) != 0)
        {
            server_msg_stream << "Can't get hostname" << std::endl;
            print_line(msg_type_error, server_msg_stream);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int SocketController::read_data(char *data_to_send, int data_size)
{
    int msg_size = read(*_client_socket_ptr, data_to_send, data_size);
    return msg_size;
}

int SocketController::write_data(char *data_to_send, int data_size)
{
    int msg_size = write(*_client_socket_ptr, data_to_send, data_size);
    return msg_size;
}

char *SocketController::get_hostname()
{
    return _hostname;
}

char *SocketController::get_server_info()
{
    return _server_info;
}

int SocketController::get_client_port()
{
    return ntohs(_client_address.sin_port);
}

const char *SocketController::get_client_addr()
{
    return inet_ntop(AF_INET, &_client_address.sin_addr, _client_address_buf, sizeof(_client_address_buf) / sizeof(_client_address_buf[0]));
}
