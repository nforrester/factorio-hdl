#include "util.h"

#include <fstream>
#include <cassert>

std::string read_file(std::string const & filename)
{
    std::ifstream f(filename);
    assert(f.good());
    return std::string { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
}
