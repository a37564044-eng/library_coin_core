#include "blockchain.h"
#include "genesis.h"
#include "consensus/pow.h"

#include <iostream>

int main() {
    std::cout << "=== LARB CHAINWORK TEST ===\n";

    constexpr std::uint32_t difficulty = 3;

    larb::Block genesis =
        larb::Genesis::create();

    larb::Blockchain chain(
        genesis,
        difficulty
    );

    if (chain.chain_work() != 0) {
        std::cerr << "Genesis work: FAIL\n";
        return 1;
    }

    std::cout << "Genesis work: OK\n";

    larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {"CHAINWORK-TX1"},
            1700000001,
            difficulty
        );

    if (!chain.add_block(block1)) {
        std::cerr << "Add block 1: FAIL\n";
        return 1;
    }

    if (chain.chain_work() != 4096) {
        std::cerr << "Block 1 work: FAIL\n";
        return 1;
    }

    std::cout << "Block 1 work: OK\n";

    larb::Block block2 =
        larb::mine_block(
            1,
            block1.hash(),
            {"CHAINWORK-TX2"},
            1700000002,
            difficulty
        );

    if (!chain.add_block(block2)) {
        std::cerr << "Add block 2: FAIL\n";
        return 1;
    }

    if (chain.chain_work() != 8192) {
        std::cerr << "Block 2 cumulative work: FAIL\n";
        return 1;
    }

    std::cout << "Block 2 cumulative work: OK\n";

    if (!chain.is_valid()) {
        std::cerr << "Chain validation: FAIL\n";
        return 1;
    }

    std::cout << "Chain validation: OK\n";

    std::cout << "Chain work: "
              << static_cast<unsigned long long>(
                     chain.chain_work())
              << '\n';

    std::cout
        << "=== ALL CHAINWORK TESTS PASSED ===\n";

    return 0;
}
