#pragma once
#include <cstdint>
namespace larb { constexpr std::int64_t COIN 
= 100'000'000; constexpr std::int64_t 
MAX_MONEY = 21'000'000 * COIN; constexpr 
std::uint64_t HALVING_INTERVAL = 210'000; 
constexpr std::int64_t INITIAL_BLOCK_REWARD = 
50 * COIN; std::int64_t 
get_block_reward(std::uint64_t height);
} // namespace larb



namespace larb {
constexpr std::uint32_t INITIAL_POW_DIFFICULTY = 3;
}
