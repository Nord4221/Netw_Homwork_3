#include "client_msg_analyzer.h"
#include <stdlib.h>

int MsgAnalyzer::find_command(std::stringstream  &msg)
{
    std::string msg_str = msg.str();
    if (msg.str().find(COMMAND_NAME_EXIT) != std::string::npos)
    {
        msg << "Found exit command" ;
        return _state_exit;
    }
    else if (msg.str().find(COMMAND_NAME_FILE_RECIEVE) != std::string::npos)
    {
        msg << "Found file command" ;
        char file_size[20];
        int file_name_pos = msg_str.find(COMMAND_DIVIDER_SYMBOL, msg_str.find(COMMAND_NAME_FILE_RECIEVE)) + COMMAND_DIVIDER_SIZE;
        int file_size_pos = msg_str.find(COMMAND_DIVIDER_SYMBOL, file_name_pos) + COMMAND_DIVIDER_SIZE;
        int payload_pos = msg_str.find(COMMAND_DIVIDER_SYMBOL, file_size_pos) + COMMAND_DIVIDER_SIZE;
        int name_pos = 0;
        int file_pos = 0;
        for(int i = file_name_pos; i < (file_size_pos - COMMAND_DIVIDER_SIZE); i++)
            _file_from_server->_file_name[name_pos++] = msg_str[i];
        for(int i = file_size_pos; i < (payload_pos - COMMAND_DIVIDER_SIZE); i++)
            file_size[file_pos++] = msg_str[i];
        _file_from_server->_file_size = atoi(file_size);
        _header_size = payload_pos;
        return _state_file_read;
    }
    else
        return _state_print_echo;
}

int MsgAnalyzer::get_file_size()
{
    return  _file_from_server->_file_size;
}

int MsgAnalyzer::get_header_size()
{
    return _header_size;
}

char *MsgAnalyzer::get_file_name()
{
    return  _file_from_server->_file_name;
}


