#include "util.h"

#include <fstream>

std::string read_file(std::string const & filename)
{
    std::ifstream f(filename);
    if (!f.good())
    {
        throw std::runtime_error("Unable to open '" + filename + "'");
    }
    return std::string { std::istreambuf_iterator<char>(f),
                         std::istreambuf_iterator<char>() };
}
