#pragma once

// TODO audit use of assert throughout codebase and then remove this.
#undef NDEBUG
#include <cassert>

#include <string>

#define FILE_LINE std::string(__FILE__ ":" + std::to_string(__LINE__))

std::string read_file(std::string const & filename);

size_t constexpr max_signals_per_constant_combinator = 18;
