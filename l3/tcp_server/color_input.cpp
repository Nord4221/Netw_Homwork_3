#include "color_input.h"

void print_line(char msg_type, std::stringstream &msg)
{
    if (msg_type == msg_type_ordinary)
        std::cout << "\x1b[1;30m" << msg.str() << "\x1b[1;30m" << "\n" << std::endl;
    else if (msg_type == msg_type_succes)
        std::cout << "\x1b[1;32m" << msg.str() << "\x1b[1;30m" << "\n" << std::endl;
    else if (msg_type == msg_type_error)
        std::cout << "\x1b[1;31m" << msg.str() << "\x1b[1;30m" << "\n" << std::endl;

    msg.str("");
}
