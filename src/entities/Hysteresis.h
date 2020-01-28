#pragma once

#include "src/Composite.h"

class Hysteresis: public Composite
{
public:
    /* Actual dead band width is (dead_band_half_width * 2 - 1) */
    Hysteresis(Factorio & factorio,
               SignalId in,
               SignalId out,
               SignalValue dead_band_center_point,
               SignalValue dead_band_half_width,
               WireColor interface_color);
};
