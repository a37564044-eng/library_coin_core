#include "src/blockchain.h"
#include "src/genesis.h"
#include "src/consensus/pow.h"
#include "src/consensus/constants.h"

#include <iostream>

int main() {
    std::cout << "=== LARB BLOCKCHAIN TEST ===\n";

    larb::Block genesis =
        larb::Genesis::create();

    larb::Blockchain chain(genesis);

    if (chain.size() != 1) {
        std::cerr << "Genesis chain size: FAIL\n";
        return 1;
    }

    std::cout << "Genesis chain size: OK\n";

    larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {"TX1"},
            2,
            larb::INITIAL_POW_DIFFICULTY
        );

    if (!chain.add_block(block1)) {
        std::cerr << "Add mined block 1: FAIL\n";
        return 1;
    }

    std::cout << "Add mined block 1: OK\n";

    larb::Block block2 =
        larb::mine_block(
            1,
            block1.hash(),
            {"TX2"},
            3,
            larb::INITIAL_POW_DIFFICULTY
        );

    if (!chain.add_block(block2)) {
        std::cerr << "Add mined block 2: FAIL\n";
        return 1;
    }

    std::cout << "Add mined block 2: OK\n";

    if (chain.size() != 3) {
        std::cerr << "Chain size: FAIL\n";
        return 1;
    }

    std::cout << "Chain size: OK\n";

    if (!chain.is_valid()) {
        std::cerr << "Chain validation: FAIL\n";
        return 1;
    }

    std::cout << "Chain validation: OK\n";

    larb::Block invalid_block(
        1,
        "WRONG_PREVIOUS_HASH",
        {"TX3"},
        4,
        0
    );

    if (chain.add_block(invalid_block)) {
        std::cerr << "Reject invalid previous hash: FAIL\n";
        return 1;
    }

    std::cout << "Reject invalid previous hash: OK\n";

    std::cout << "=== ALL BLOCKCHAIN TESTS PASSED ===\n";

    return 0;
}
