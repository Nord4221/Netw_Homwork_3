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

    if(init_ssl_wrapper() == EXIT_FAILURE)
        return EXIT_FAILURE;

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
int SocketController::init_ssl_wrapper()
{
    // OpenSSL initialization.
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    // Get TLS methods from library.
    const SSL_METHOD *meth = TLS_server_method();

    SSL_CTX *ctx = SSL_CTX_new(meth);

    if (SSL_CTX_use_certificate_file(ctx, "server.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    _ssl = SSL_new(ctx);

    if (!_ssl)
    {
        server_msg_stream << "Error creating SSL." << std::endl;
        print_line(msg_type_error, server_msg_stream);
        log_ssl();
        return EXIT_FAILURE;
    }

    // Place in connection data structure
    // real socket decriptor
    // You can get decriptor back by `SSL_get_fd(ssl)`.
    SSL_set_fd(_ssl, *_client_socket_ptr);
    // Do handshake SSL/TLS.
    int err = SSL_accept(_ssl);
    if (err <= 0)
    {
        server_msg_stream
            << "Error creating SSL connection.  err = " << err << std::endl;
        print_line(msg_type_error, server_msg_stream);
        log_ssl();
        return EXIT_FAILURE;
    }
    server_msg_stream << "SSL connection using " << SSL_get_cipher(_ssl) << std::endl;
    print_line(msg_type_error, server_msg_stream);
    return EXIT_SUCCESS;
}

void SocketController::log_ssl()
{
    for (int err = ERR_get_error(); err; err = ERR_get_error())
    {
        char *str = ERR_error_string(err, 0);
        if (!str) return;
        std::cerr << str << std::endl;
    }
}

int SocketController::read_data(char *data_buffer, int data_size)
{
    // Reading in buffer through SSL.
    int msg_size = SSL_read(_ssl, data_buffer, data_size - 1);
    get_ssl_rw_error(msg_size);
    return msg_size;
}

int SocketController::write_data(char *data_to_send, int data_size)
{
    int msg_size = SSL_write(_ssl, data_to_send, data_size);
    get_ssl_rw_error(msg_size);
    return msg_size;
}

bool SocketController::get_ssl_rw_error(int msg_length)
{
    if (msg_length < 0)
    {
        int err = SSL_get_error(_ssl, msg_length);
        switch (err)
        {
            // It's not mistakes.
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_READ:
            break;
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
            default:
            {
                server_msg_stream << "SSL error or message size less than zero" << std::endl;
                print_line(msg_type_error, server_msg_stream);
                return false;
            }
        }
    }
    else
        return true;
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
