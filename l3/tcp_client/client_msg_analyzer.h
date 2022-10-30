#ifndef MSG_ANALYZER_H
#define MSG_ANALYZER_H
/*
 *Responce for analyzing commands in messages
*/
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

//const int command_id_no_commands = 0;
//const int command_id_exit = 1;
const std::string COMMAND_NAME_EXIT = "exit";
const std::string COMMAND_NAME_FILE_RECIEVE = "file";
const std::string COMMAND_DIVIDER_SYMBOL = "/";
const int COMMAND_DIVIDER_SIZE = 1;

enum States
{
    _state_get_user_msg,
    _state_recieve_with_command_check,
    _state_recieve_without_command_check,
    _state_print_echo,
    _state_file_read,
    _state_exit
};

struct FileToRead
{
    int _file_size;
    char _file_name[255];
    std::ofstream _file_stream;
};

class MsgAnalyzer
{
public:
    MsgAnalyzer(){ _file_from_server = new FileToRead;}
    int find_command(std::stringstream  &msg);
    int get_file_size();
    int get_header_size();
    char* get_file_name();
    FileToRead *_file_from_server;
private:
    int _header_size = 0;
};


#endif // MSG_ANALYZER_H
