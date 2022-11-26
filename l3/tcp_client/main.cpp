#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#   define ioctl ioctlsocket
#else
extern "C"
{
#   include <netinet/tcp.h>
#   include <sys/ioctl.h>
// #   include <fcntl.h>
}
#endif

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#include "client_msg_analyzer.h"
#include "socket_controller.h"
using namespace std::chrono_literals;

const auto MAX_RECV_BUFFER_SIZE = 256;
const auto MIN_MSG_SIZE = 20;

int main(int argc, const char * const argv[])
{
    MsgAnalyzer *msg_analyzer = new MsgAnalyzer();
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }
    SocketController* socket_controller = new SocketController();
    if(socket_controller->client_socket_init(argv[1],std::stoi(argv[2])) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    std::string request;
    std::vector<char> buffer;
    buffer.resize(MAX_RECV_BUFFER_SIZE);

    std::cout << "Connected to \"" << socket_controller->get_hostname() << "\"..." << std::endl;

    std::cout << "Waiting for the user input..." << std::endl;
    int state_of_client = _state_get_user_msg;
    std::stringstream command_stream;
    long recv_bytes = 0;
    int bytes_enough_recieved = 0;
    int read_pos = 0;
    while (true)
    {
        switch (state_of_client)
        {
            case _state_get_user_msg:
                std::cout << "> " << std::flush;
                if (!std::getline(std::cin, request)) break;

                std::cout
                    << "Sending request: \"" << request << "\"..."
                    << std::endl;

                request += "\r\n";

                if (!socket_controller->write_data(request))
                {
                    std::cerr << "Request error" << std::endl;
                    return EXIT_FAILURE;
                }

                std::cout
                    << "Request was sent, reading response..."
                    << std::endl;
                std::this_thread::sleep_for(2ms);
                state_of_client = _state_recieve_with_command_check;
            break;
            case _state_recieve_with_command_check:
                while (true)
                {
                    recv_bytes = socket_controller->read_data(buffer.data(), buffer.size() - 1);

                    std::cout
                        << recv_bytes
                        << " was received..."
                        << std::endl;

                    if (recv_bytes > 0)
                    {
                        buffer[recv_bytes] = '\0';
                        command_stream << std::string(buffer.begin(), std::next(buffer.begin(), recv_bytes));
                        std::cout << command_stream.str();
                        state_of_client = msg_analyzer->find_command(command_stream);
                        std::cout << "Current state " << state_of_client << std::endl;
                        if(state_of_client != _state_get_user_msg)
                            break;
                        else
                            continue;
                    }
                    else if (-1 == recv_bytes)
                    {
                        if (EINTR == errno) continue;
                        if (0 == errno) break;
                        // std::cerr << errno << ": " << sock_wrap.get_last_error_string() << std::endl;
                        state_of_client = _state_get_user_msg;
                        break;
                    }
                    recv_bytes = 0;
                    state_of_client = _state_get_user_msg;
                    break;
                }
            break;
            case _state_recieve_without_command_check:
                while (true)
                {
                    recv_bytes = socket_controller->read_data(buffer.data(), buffer.size() - 1);
                    bytes_enough_recieved += recv_bytes;
                    if (bytes_enough_recieved >= MIN_MSG_SIZE)
                    {
                        buffer[recv_bytes] = '\0';
                        command_stream << std::string(buffer.begin(), std::next(buffer.begin(), recv_bytes));
                        read_pos = 0;
                        state_of_client = _state_file_read;
                    }
                }
            break;
            case _state_print_echo:
                std::cout << "------------\n" << std::string(buffer.begin(), std::next(buffer.begin(), recv_bytes)) << std::endl;
                recv_bytes = 0;
                state_of_client = _state_get_user_msg;
            break;
            case _state_file_read:
                if (!msg_analyzer->_file_from_server->_file_stream.is_open())
                {
                    std::cout << " Opened/created file - " << msg_analyzer->_file_from_server->_file_name << "\n\r"
                               << " File size - " << msg_analyzer->_file_from_server->_file_size << std::endl;
                    msg_analyzer->_file_from_server->_file_stream.open(msg_analyzer->_file_from_server->_file_name);
                    read_pos = msg_analyzer->get_header_size();
                }
                if (msg_analyzer->_file_from_server->_file_stream.is_open())
                {
                    int shank_size = msg_analyzer->_file_from_server->_file_size;
                    if (shank_size > (recv_bytes - read_pos))
                        for(; read_pos < recv_bytes; read_pos++)
                        {
                            msg_analyzer->_file_from_server->_file_stream.put(buffer[read_pos]);
                            msg_analyzer->_file_from_server->_file_size--;
                        }
                    else
                        for(; msg_analyzer->_file_from_server->_file_size > 0; read_pos++)
                        {
                            msg_analyzer->_file_from_server->_file_stream.put(buffer[read_pos]);
                            msg_analyzer->_file_from_server->_file_size--;
                        }
                }
                else
                {
                    std::cout << "File not opened";
                    break;
                }
                if(msg_analyzer->_file_from_server->_file_size <= 0)
                {
                    std::cout << "Closed file - " << msg_analyzer->_file_from_server->_file_name;
                    msg_analyzer->_file_from_server->_file_stream.close();
                    state_of_client = _state_get_user_msg;
                    buffer[0] = '\0';
                    buffer.clear();
                    bytes_enough_recieved = 0;
                }
            break;
        }
    }

    return EXIT_SUCCESS;
}

