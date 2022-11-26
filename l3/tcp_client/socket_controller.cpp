#include "socket_controller.h"

std::stringstream server_msg_stream;

SocketController::SocketController()
{

}

SocketController::~SocketController()
{
    close(*_client_socket_ptr);
    _sock_wrap_ptr->~SocketWrapper();
}

int SocketController::client_socket_init(const char* server_ip, int server_port)
{
    _sock_wrap_ptr = new socket_wrapper::SocketWrapper;
    _client_socket_ptr = new socket_wrapper::Socket {AF_INET, SOCK_STREAM, IPPROTO_TCP};

    if (!(*_client_socket_ptr))
    {
        server_msg_stream << _sock_wrap_ptr->get_last_error_string() << std::endl;
        print_line(msg_type_error, server_msg_stream);
        return EXIT_FAILURE;
    }

    _hostname = new std::string{ server_ip };
    const struct hostent *remote_host { gethostbyname(_hostname->c_str()) };

    server_msg_stream << "Starting client on the ip::port :" << *_hostname << ":" << server_port ;
    print_line(msg_type_ordinary, server_msg_stream);

    _server_address =
    {
        .sin_family = AF_INET,
        .sin_port = htons(server_port)
    };

    _server_address.sin_addr.s_addr = *reinterpret_cast<const in_addr_t*>(remote_host->h_addr);

    if (connect(*_client_socket_ptr, reinterpret_cast<const sockaddr* const>(&_server_address), sizeof(_server_address)) != 0)
    {
        server_msg_stream << _sock_wrap_ptr->get_last_error_string() << std::endl;
        print_line(msg_type_error, server_msg_stream);
        return EXIT_FAILURE;
    }

    if(init_ssl_wrapper() == EXIT_FAILURE)
        return EXIT_FAILURE;

    // Put the socket in non-blocking mode:
    const IoctlType flag = 1;

    if (ioctl(*_client_socket_ptr, FIONBIO, const_cast<IoctlType*>(&flag)) < 0)
    {
        server_msg_stream << _sock_wrap_ptr->get_last_error_string() << std::endl;
        print_line(msg_type_error, server_msg_stream);
        return EXIT_FAILURE;
    }

    // Disable Naggles's algorithm.
    if (setsockopt(*_client_socket_ptr, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&flag), sizeof(flag)) < 0)
    {
        server_msg_stream << _sock_wrap_ptr->get_last_error_string() << std::endl;
        print_line(msg_type_error, server_msg_stream);
        return EXIT_FAILURE;
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
    const SSL_METHOD *meth = TLS_client_method();

    SSL_CTX *ctx = SSL_CTX_new(meth);
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
    int err = SSL_connect(_ssl);
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

bool SocketController::write_data(const std::string &request)
{
    ssize_t bytes_count = 0;
    size_t req_pos = 0;
    auto const req_buffer = &(request.c_str()[0]);
    auto const req_length = request.length();

    while (true)
    {
        if ((bytes_count = SSL_write(_ssl, req_buffer + req_pos, req_length - req_pos)) < 0)
        {
            if (EINTR == errno) continue;
        }
        else
        {
            if (!bytes_count) break;

            req_pos += bytes_count;

            if (req_pos >= req_length)
            {
                break;
            }
        }
    }

    get_ssl_rw_error(bytes_count);
    return true;
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

std::string *SocketController::get_hostname()
{
    return _hostname;
}

