#pragma once

#include <string>

std::string zlib_decompress(std::string const & compressed);
std::string zlib_compress(std::string const & plain);
