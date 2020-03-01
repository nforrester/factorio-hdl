#pragma once

#include "src/debug.h"

#include <algorithm>

class Bits
{
private:
    using Block = uint64_t;
    static constexpr int_fast8_t bits_per_block = sizeof(Block) * 8;
    static_assert(bits_per_block == 64);

public:
    Bits(size_t size): _size(size), _storage(size/bits_per_block + 1, 0)
    {
    }

    void clear()
    {
        for (Block & block : _storage)
        {
            block = 0;
        }
    }

    void set(size_t bit)
    {
        size_t block_num = bit / bits_per_block;
        size_t subbit_num = bit % bits_per_block;
        _storage[block_num] |= static_cast<Block>(1) << subbit_num;
    }

    bool get(size_t bit)
    {
        size_t block_num = bit / bits_per_block;
        size_t subbit_num = bit % bits_per_block;
        return _storage[block_num] & (static_cast<Block>(1) << subbit_num);
    }

    Bits const & operator&=(Bits const & other)
    {
        for (size_t block_num = 0; block_num < _storage.size(); ++block_num)
        {
            _storage[block_num] &= other._storage[block_num];
        }
        return *this;
    }

    bool any()
    {
        for (size_t block_num = 0; block_num < _storage.size(); ++block_num)
        {
            if (_storage[block_num])
            {
                return true;
            }
        }
        return false;
    }

    void which(std::vector<size_t> & output)
    {
        size_t bits_in_blocks_not_already_done = _size;
        for (size_t block_num = 0; block_num < _storage.size(); ++block_num)
        {
            Block block = _storage[block_num];
            if (block == 0)
            {
                continue;
            }

            int_fast8_t const bits_in_this_block =
                std::min<size_t>(bits_in_blocks_not_already_done, 64u);

            int_fast8_t bit_num = 0;
            int_fast8_t one_plus_upper_bound_for_next_high_bit = bits_in_this_block;
            while (bit_num < bits_in_this_block)
            {
                int_fast8_t bits_left = one_plus_upper_bound_for_next_high_bit - bit_num;


                Block half_mask = ((static_cast<Block>(1) << (bits_left / 2)) - 1) << bit_num;
                Block full_mask = std::numeric_limits<Block>::max() << bit_num;

                if ((block & full_mask) == 0)
                {
                    break;
                }

                if ((block & half_mask) == 0)
                {
                    bit_num += bits_left / 2;
                }
                else
                {
                    one_plus_upper_bound_for_next_high_bit = bits_left / 2 + bit_num;
                }
                if (one_plus_upper_bound_for_next_high_bit - 1 == bit_num)
                {
                    output.push_back(bit_num + bits_per_block * block_num);
                    ++bit_num;
                    one_plus_upper_bound_for_next_high_bit = bits_in_this_block;
                }
            }
            bits_in_blocks_not_already_done -= bits_per_block;
        }
    }

private:
    size_t _size;
    std::vector<Block> _storage;
};
