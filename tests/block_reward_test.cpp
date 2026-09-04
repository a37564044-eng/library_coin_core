#include "src/consensus/constants.h"
#include "src/transaction.h"

#include <iostream>

int main() {
    std::cout << "=== LARB BLOCK REWARD TEST ===\n";

    const std::uint64_t heights[] = {
        0,
        210'000,
        6'720'000,
        6'930'000
    };

    const std::int64_t expected[] = {
        5'000'000'000,
        2'500'000'000,
        1,
        0
    };

    for (int i = 0; i < 4; ++i) {
        const std::int64_t reward =
            larb::get_block_reward(heights[i]);

        if (reward != expected[i]) {
            std::cerr << "Height "
                      << heights[i]
                      << ": FAIL\n";
            return 1;
        }

        if (reward > 0) {
            const larb::Transaction coinbase =
                larb::Transaction::coinbase(
                    reward,
                    "LARB-MINER"
                );

            if (coinbase.inputs.size() != 0 ||
                coinbase.outputs.size() != 1 ||
                coinbase.outputs[0].amount != reward) {
                std::cerr << "Coinbase at height "
                          << heights[i]
                          << ": FAIL\n";
                return 1;
            }
        }

        std::cout << "Height "
                  << heights[i]
                  << ": "
                  << reward
                  << " satoshi: OK\n";
    }

    std::cout << "=== ALL BLOCK REWARD TESTS PASSED ===\n";

    return 0;
}
