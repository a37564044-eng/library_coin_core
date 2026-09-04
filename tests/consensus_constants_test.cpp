#include "consensus/constants.h"

#include <cstdint>
#include <iostream>

int main() {
    bool ok = true;

    std::cout << "=== LARB CONSENSUS CONSTANTS TEST ===\n";

    if (larb::COIN != 100'000'000) {
        std::cout << "COIN: FAIL\n";
        ok = false;
    } else {
        std::cout << "COIN: OK\n";
    }

    if (larb::MAX_MONEY != 21'000'000LL * 100'000'000LL) {
        std::cout << "MAX_MONEY: FAIL\n";
        ok = false;
    } else {
        std::cout << "MAX_MONEY: OK\n";
    }

    if (larb::HALVING_INTERVAL != 210'000) {
        std::cout << "HALVING_INTERVAL: FAIL\n";
        ok = false;
    } else {
        std::cout << "HALVING_INTERVAL: OK\n";
    }

    if (larb::INITIAL_BLOCK_REWARD != 50LL * larb::COIN) {
        std::cout << "INITIAL_BLOCK_REWARD: FAIL\n";
        ok = false;
    } else {
        std::cout << "INITIAL_BLOCK_REWARD: OK\n";
    }

    if (larb::INITIAL_POW_DIFFICULTY != 3) {
        std::cout << "INITIAL_POW_DIFFICULTY: FAIL\n";
        ok = false;
    } else {
        std::cout << "INITIAL_POW_DIFFICULTY: OK\n";
    }

    if (larb::get_block_reward(0) != 50LL * larb::COIN) {
        std::cout << "Reward height 0: FAIL\n";
        ok = false;
    } else {
        std::cout << "Reward height 0: OK\n";
    }

    if (larb::get_block_reward(209'999) != 50LL * larb::COIN) {
        std::cout << "Reward height 209999: FAIL\n";
        ok = false;
    } else {
        std::cout << "Reward height 209999: OK\n";
    }

    if (larb::get_block_reward(210'000) != 25LL * larb::COIN) {
        std::cout << "Reward height 210000: FAIL\n";
        ok = false;
    } else {
        std::cout << "Reward height 210000: OK\n";
    }

    if (larb::get_block_reward(420'000) != 1'250'000'000LL) {
        std::cout << "Reward height 420000: FAIL\n";
        ok = false;
    } else {
        std::cout << "Reward height 420000: OK\n";
    }

    if (!ok) {
        std::cout << "=== CONSENSUS CONSTANTS TEST FAILED ===\n";
        return 1;
    }

    std::cout << "=== ALL CONSENSUS CONSTANTS TESTS PASSED ===\n";
    return 0;
}
