#pragma once

#include <iostream>

#define debug_level 0

#define info(n) if (n > debug_level) {} else std::cout
#define debug(n) if (n > debug_level) {} else std::cerr
