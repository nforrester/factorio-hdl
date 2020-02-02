#pragma once

#include<utility>

int compute_final_side_length(int length_of_curve);

std::pair<int, int>
compute_position_along_hilbert_curve(int final_side_length, int distance_along_curve);
