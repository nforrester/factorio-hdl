#include "hilbert_curve.h"

int compute_final_side_length(int length_of_curve)
{
    int final_side_length = 1;
    while (final_side_length * final_side_length < length_of_curve)
    {
        final_side_length *= 2;
    }
    return final_side_length;
}

std::pair<int, int>
compute_position_along_hilbert_curve(int final_side_length, int distance_along_curve) {
    int x = 0;
    int y = 0;

    for (int side_length = 1; side_length < final_side_length; side_length *= 2)
    {
        bool const flip_x = distance_along_curve & 2;
        bool const flip_y = flip_x != (distance_along_curve & 1);

        if (!flip_y)
        {
            if (flip_x)
            {
                x = (side_length - 1) - x;
                y = (side_length - 1) - y;
            }

            int temp = x;
            x = y;
            y = temp;
        }

        x += (flip_x ? side_length : 0);
        y += (flip_y ? side_length : 0);
        distance_along_curve /= 4;
    }

    return std::make_pair(x, y);
}
