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
const std::string command_name_exit = "exit";
const std::string command_name_file_recieve = "file";
const std::string command_divider = "/";
const int command_divider_size = 1;

enum States
{
    _state_echo,
    _state_file_read,
    _state_exit
};

struct FileToWrite
{
    int _file_size;
    char _file_name[255];
    std::ofstream _file_stream;
};

class MsgAnalyzer
{
public:
    MsgAnalyzer(){ _file_from_server = new FileToWrite;}
    int find_command(std::stringstream  &msg);
    int get_file_size();
    int get_header_size();
    char* get_file_name();
    FileToWrite *_file_from_server;
private:
    int _header_size = 0;
};


#endif // MSG_ANALYZER_H
