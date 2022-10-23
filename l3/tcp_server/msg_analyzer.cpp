#include "msg_analyzer.h"
#include <stdlib.h>

int MsgAnalyzer::find_command(std::stringstream  &msg)
{
    std::string msg_str = msg.str();
    if (msg.str().find(command_name_exit) != std::string::npos)
    {
        msg << "Found exit command" ;
        return _state_exit;
    }
    else if (msg.str().find(command_name_file_recieve) != std::string::npos)
    {
        msg << "Found file command" ;
        char file_size[20];
        int file_name_pos = msg_str.find(command_divider, msg_str.find(command_name_file_recieve)) + command_divider_size;
        int file_size_pos = msg_str.find(command_divider, file_name_pos) + command_divider_size;
        int payload_pos = msg_str.find(command_divider, file_size_pos) + command_divider_size;
        int name_pos = 0;
        int file_pos = 0;
        for(int i = file_name_pos; i < (file_size_pos - command_divider_size); i++)
            _file_from_server->_file_name[name_pos++] = msg_str[i];
        for(int i = file_size_pos; i < (payload_pos - command_divider_size); i++)
            file_size[file_pos++] = msg_str[i];
        _file_from_server->_file_size = atoi(file_size);
        _header_size = payload_pos;
        return _state_file_read;
    }
    else
    return _state_echo;
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


