#pragma once

#include <cstdint>

// TODO make rollover defined behavior
using SignalValue = int32_t;

using SignalId = uint_fast16_t;
SignalId constexpr num_signals = 60 + 37 + 60 + 56 + 48;

namespace LogicSignal
{
    SignalId constexpr max_id = std::numeric_limits<SignalId>::max();

    SignalId constexpr everything = max_id - 1;
    SignalId constexpr anything = everything - 1;
    SignalId constexpr each = anything - 1;
    SignalId constexpr constant = each - 1;

    SignalId constexpr min_id = constant;
}

static_assert(num_signals <= LogicSignal::min_id);
