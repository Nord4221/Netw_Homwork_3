#ifndef COLOR_INPUT_H
#define COLOR_INPUT_H
/*
 *Responce for information output
*/
#include <sstream>
#include <iostream>
#include <string>

const char msg_type_error = 0;
const char msg_type_succes = 1;
const char msg_type_ordinary = 2;

void print_line(char msg_type, std::stringstream  &msg);


#endif // COLOR_INPUT_H
