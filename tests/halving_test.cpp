#include "src/consensus/constants.h"

#include <cstdint>
#include <iostream>

int main() {
    std::cout << "=== LARB HALVING TEST ===\n";

    const std::uint64_t interval = larb::HALVING_INTERVAL;

    if (larb::get_block_reward(0) != 5'000'000'000) {
        std::cerr << "Block 0: FAIL\n";
        return 1;
    }

    if (larb::get_block_reward(interval) != 2'500'000'000) {
        std::cerr << "Halving 1: FAIL\n";
        return 1;
    }

    if (larb::get_block_reward(interval * 2) != 1'250'000'000) {
        std::cerr << "Halving 2: FAIL\n";
        return 1;
    }

    if (larb::get_block_reward(interval * 10) <= 0) {
        std::cerr << "Halving 10: FAIL\n";
        return 1;
    }

    // 32 kali halving = 1 satoshi
    if (larb::get_block_reward(interval * 32) != 1) {
        std::cerr << "Final 1 satoshi: FAIL\n";
        return 1;
    }

    // Setelah reward 1 satoshi, menjadi 0
    if (larb::get_block_reward(interval * 33) != 0) {
        std::cerr << "Post-final reward: FAIL\n";
        return 1;
    }

    std::cout << "Block 0: "
              << larb::get_block_reward(0)
              << " satoshi\n";

    std::cout << "Block "
              << interval
              << ": "
              << larb::get_block_reward(interval)
              << " satoshi\n";

    std::cout << "Block "
              << interval * 32
              << ": "
              << larb::get_block_reward(interval * 32)
              << " satoshi\n";

    std::cout << "Block "
              << interval * 33
              << ": "
              << larb::get_block_reward(interval * 33)
              << " satoshi\n";

    std::cout << "=== ALL HALVING TESTS PASSED ===\n";

    return 0;
}
