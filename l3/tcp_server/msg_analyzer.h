#ifndef MSG_ANALYZER_H
#define MSG_ANALYZER_H
/*
 *Responce for analyzing commands in messages
*/
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

//const int command_id_no_commands = 0;
//const int command_id_exit = 1;
const std::string COMMAND_NAME_EXIT = "exit";
const std::string COMMAND_NAME_FILE_REQUEST = "file";
const std::string COMMAND_DIVIDER_SYMBOL = "/";
const int COMMAND_DIVIDER_SIZE = 1;

enum States
{
    _state_recieve,
    _state_echo,
    _state_file_requested,
    _state_file_requested_error,
    _state_file_send,
    _state_exit
};

struct FileToWrite
{
    int _file_size;
    char _file_name[255];
    std::ifstream _file_stream;
};

class MsgAnalyzer
{
public:
    MsgAnalyzer(){ _file_from_server = new FileToWrite;}
    int find_command(std::stringstream  &msg);
    int get_file_size();
    int get_header_size();
    char* get_file_name();
    int get_file_size_from_filesystem();
    int get_file_size_from_counter();
    FileToWrite *_file_from_server;
private:
    int _header_size = 0;
};


#endif // MSG_ANALYZER_H
