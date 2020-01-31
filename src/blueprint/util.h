#pragma once

#include "src/signals.h"

/* This is a header file for blueprint stuff that wants to see more common use
 * by things that don't want json.hpp. These things are implemented in
 * Blueprint.cpp */

SignalId get_signal_id_from_lower_case(std::string const & name);
