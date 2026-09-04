#include "consensus/pow.h"

#include <iostream>

int main() {
    std::cout << "=== LARB PROOF OF WORK TEST ===\n";

    larb::Block block(
        1,
        "previous",
        {"TX1"},
        100,
        0
    );

    if (!larb::validate_proof_of_work(block, 0)) {
        std::cerr << "Difficulty 0: FAIL\n";
        return 1;
    }

    std::cout << "Difficulty 0: OK\n";

    const std::uint32_t difficulty = 3;

    larb::Block mined = larb::mine_block(
        1,
        "previous",
        {"TX1"},
        100,
        difficulty
    );

    if (!larb::validate_proof_of_work(
            mined,
            difficulty)) {
        std::cerr << "Mined block PoW: FAIL\n";
        return 1;
    }

    std::cout << "Mined block PoW: OK\n";
    std::cout << "Nonce: "
              << mined.header().nonce
              << "\n";
    std::cout << "Hash: "
              << mined.hash()
              << "\n";

    std::cout << "=== ALL POW TESTS PASSED ===\n";

    return 0;
}
