#pragma once

#include "src/host/signals.h"

/* This is a header file for blueprint stuff that wants to see more common use
 * by things that don't want json.hpp. These things are implemented in
 * Blueprint.cpp */

SignalId get_signal_id_from_lower_case(std::string const & name);
std::string get_signal_name_lower_case(SignalId id);
