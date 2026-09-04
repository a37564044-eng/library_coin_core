#include "src/blockchain.h"
#include "src/genesis.h"
#include "src/consensus/pow.h"
#include "src/consensus/constants.h"

#include <iostream>

int main() {
    std::cout << "=== LARB BLOCKCHAIN TAMPER TEST ===\n";

    larb::Block genesis =
        larb::Genesis::create();

    larb::Blockchain chain(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {"TX1"},
            2,
            larb::INITIAL_POW_DIFFICULTY
        );

    larb::Block block2 =
        larb::mine_block(
            1,
            block1.hash(),
            {"TX2"},
            3,
            larb::INITIAL_POW_DIFFICULTY
        );

    if (!chain.add_block(block1)) {
        std::cerr << "Add block 1: FAIL\n";
        return 1;
    }

    std::cout << "Add block 1: OK\n";

    if (!chain.add_block(block2)) {
        std::cerr << "Add block 2: FAIL\n";
        return 1;
    }

    std::cout << "Add block 2: OK\n";

    std::cout << "Initial chain: OK\n";

    if (!chain.is_valid()) {
        std::cerr << "Initial validation: FAIL\n";
        return 1;
    }

    std::cout << "Initial validation: OK\n";

    larb::Block tampered_block1(
        1,
        genesis.hash(),
        {"TX1-TAMPERED"},
        2,
        block1.header().nonce
    );

    if (tampered_block1.hash() == block1.hash()) {
        std::cerr << "Tamper hash detection: FAIL\n";
        return 1;
    }

    std::cout << "Tamper changes hash: OK\n";

    if (tampered_block1.hash() ==
        block2.header().previous_hash) {

        std::cerr << "Broken chain detection: FAIL\n";
        return 1;
    }

    std::cout << "Broken chain relationship: OK\n";

    std::cout << "=== ALL TAMPER TESTS PASSED ===\n";

    return 0;
}
