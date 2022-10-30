#include "msg_analyzer.h"
#include <stdlib.h>

int MsgAnalyzer::find_command(std::stringstream  &msg)
{
    std::string msg_str = msg.str();
    if (msg.str().find(COMMAND_NAME_EXIT) != std::string::npos)
    {
        msg << "Found exit command" ;
        return _state_exit;
    }
    else if (msg.str().find(COMMAND_NAME_FILE_REQUEST) != std::string::npos)
    {
        msg << "Found file command" ;
        int file_name_pos = msg_str.find(COMMAND_DIVIDER_SYMBOL, msg_str.find(COMMAND_NAME_FILE_REQUEST)) + COMMAND_DIVIDER_SIZE;
        int file_size_pos = msg_str.find(COMMAND_DIVIDER_SYMBOL, file_name_pos) + COMMAND_DIVIDER_SIZE;
        int name_pos = 0;
        for(int i = file_name_pos; i < (file_size_pos - COMMAND_DIVIDER_SIZE); i++)
            _file_from_server->_file_name[name_pos++] = msg_str[i];
        _file_from_server->_file_size = get_file_size_from_filesystem(); //get_file_size_from_filesystem();
        if (_file_from_server->_file_size == -1)
            return _state_file_requested_error;
        else
            return _state_file_requested;
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

int MsgAnalyzer::get_file_size_from_filesystem()
{
    struct stat _fileStatbuff;
    int status = stat(_file_from_server->_file_name, &_fileStatbuff);
    if(status == -1)
        return -1;
    else
        return _fileStatbuff.st_size - 2;
}

int MsgAnalyzer::get_file_size_from_counter()
{
        std::ifstream file(_file_from_server->_file_name);
        size_t bytes_size = 0;

        while (!file.eof())
        {
            file.get();
            bytes_size++;
        }
        file.close();
        bytes_size--;
        return bytes_size;
}


