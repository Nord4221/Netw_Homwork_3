#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>

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

#include <chrono>
#include <thread>
#include "color_input.h"
#include "msg_analyzer.h"
#include "socket_controller.h"

const int  MAX_MSG_SIZE = 255;
const int  MSG_POSTFIX_SIZE = 2;
const auto MIN_MSG_SIZE = 20;

int main(int argc, char const *argv[])
{
    MsgAnalyzer *msg_analyzer = new MsgAnalyzer();
    std::stringstream msg_stream;

    if (argc != 2)
    {
        msg_stream << "Usage: " << argv[0] << " <port>" << std::endl;
        print_line(msg_type_error, msg_stream);
        return EXIT_FAILURE;
    }

    SocketController* socket_controller = new SocketController();
    if(socket_controller->server_socket_init(std::stoi(argv[1])) != EXIT_SUCCESS)
        return EXIT_FAILURE;
    if(socket_controller->client_socket_init() != EXIT_SUCCESS)
        return EXIT_FAILURE;
    static short packet_size = 0;
    int bytes_recieved = 0;
    int bytes_enough_recieved = 0;
    int read_pos = 0;
    char recieve_buffer[256];

    std::stringstream command_stream;
    int state_of_recieve = _state_recieve;
    char ch_obj;
    while (true)
    {
        switch (state_of_recieve)
        {
            case _state_recieve:
                // Read content into buffer from an incoming client.
                bytes_recieved = socket_controller->read_data(recieve_buffer, MAX_MSG_SIZE);
                bytes_enough_recieved += bytes_recieved;
                read_pos = 0;
                if (bytes_enough_recieved >= 20)
                {
                    command_stream << recieve_buffer;

                    if(state_of_recieve != _state_file_requested)
                        state_of_recieve = msg_analyzer->find_command(command_stream);
                }
            break;
            case _state_echo:
                recieve_buffer[bytes_recieved] = '\0';
                bytes_recieved -= MSG_POSTFIX_SIZE;
                msg_stream
                    << "Client with address "
                    << socket_controller->get_client_addr()
                    << ":" << socket_controller->get_client_port()
                    << " and name "
                    << socket_controller->get_hostname()
                    << " sent datagram "
                    << "[length = "
                    << bytes_recieved
                    << "]:\n'''\n"
                    << recieve_buffer
                    << "\n'''"
                    << std::endl;
                print_line(msg_type_succes, msg_stream);
                // Send same content back to the client ("echo").
                packet_size = socket_controller->write_data(recieve_buffer, bytes_recieved);
                recieve_buffer[0] = '\0';
                bytes_enough_recieved = 0;
                state_of_recieve = _state_recieve;
            break;
            case _state_file_requested:
                if (!msg_analyzer->_file_from_server->_file_stream.is_open())
                {
                    msg_stream << " Opened file - " << msg_analyzer->_file_from_server->_file_name << "\n\r"
                               << " File size = " << msg_analyzer->_file_from_server->_file_size << std::endl;
                    print_line(msg_type_succes, msg_stream);
                    msg_analyzer->_file_from_server->_file_stream.open(msg_analyzer->_file_from_server->_file_name);
                }
                if (msg_analyzer->_file_from_server->_file_stream.is_open())
                {
                    msg_stream.clear();
                    msg_stream << "file/copy_" << msg_analyzer->_file_from_server->_file_name << "/" << msg_analyzer->_file_from_server->_file_size << "/";
                    socket_controller->write_data(msg_stream.str().data(), msg_stream.str().size());
                    print_line(msg_type_succes, msg_stream);
                    state_of_recieve = _state_file_send;
                }
                else
                {
                    msg_stream << "Error" ;
                    state_of_recieve = _state_file_requested_error;
                }
            break;
            case _state_file_requested_error:
                msg_stream << "Can't open requested file";
                print_line(msg_type_error, msg_stream);
                state_of_recieve = _state_recieve;
            break;
            case _state_file_send:
                if (msg_analyzer->_file_from_server->_file_stream.is_open())
                {
                    msg_stream << "Start file sending successfully";
                    print_line(msg_type_succes, msg_stream);
                    if(msg_analyzer->_file_from_server->_file_size >= MIN_MSG_SIZE)
                    do
                    {
                        msg_stream.clear();
                        for(int i = 0; i < MIN_MSG_SIZE; i++)
                        {
                            ch_obj = msg_analyzer->_file_from_server->_file_stream.get();
                            msg_stream << ch_obj;
                            msg_analyzer->_file_from_server->_file_size--;
                        }
                        socket_controller->write_data(msg_stream.str().data(), MIN_MSG_SIZE);
                        print_line(msg_type_succes, msg_stream);
                    }
                    while(msg_analyzer->_file_from_server->_file_size > 0 && msg_analyzer->_file_from_server->_file_size >= MIN_MSG_SIZE);

                    int shank_size = msg_analyzer->_file_from_server->_file_size;
                    if(shank_size > 0)
                    {
                        msg_stream.clear();
                        for(int i = 0; i < shank_size; i++)
                        {
                            ch_obj = msg_analyzer->_file_from_server->_file_stream.get();
                            msg_stream << ch_obj;
                        }
                        for(int j = shank_size; j < MIN_MSG_SIZE - shank_size; j++)
                            msg_stream << '0';
                        socket_controller->write_data(msg_stream.str().data(), MIN_MSG_SIZE);
                        msg_stream << "\n";
                    }
                    msg_stream << "File - " << msg_analyzer->_file_from_server->_file_name << ", sended. \n";
                    print_line(msg_type_succes, msg_stream);
                    msg_stream.clear();
                    msg_analyzer->_file_from_server->_file_stream.close();
                    recieve_buffer[0] = '\0';
                    bytes_enough_recieved = 0;
                    state_of_recieve = _state_recieve;
                }
            break;
            case _state_exit:
                print_line(msg_type_error, msg_stream);
                msg_stream.clear();
                recieve_buffer[0] = '\0';
                bytes_enough_recieved = 0;
                return EXIT_SUCCESS;
            break;
        }
    }
    socket_controller->~SocketController();
    return EXIT_SUCCESS;
}

