#pragma once

#include <string>

#define FILE_LINE std::string(__FILE__ ":" + std::to_string(__LINE__))

std::string read_file(std::string const & filename);
