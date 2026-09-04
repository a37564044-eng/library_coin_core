#include "src/blockchain.h"
#include "src/consensus/constants.h"
#include "src/genesis.h"

#include <iostream>

int main() {
    std::cout << "=== LARB MAINNET ===\n";

    const larb::Block genesis = larb::Genesis::create();

    larb::Blockchain blockchain(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    if (!blockchain.is_valid()) {
        std::cerr << "Genesis validation: FAIL\n";
        return 1;
    }

    std::cout << "Network: LARB MAINNET\n";
    std::cout << "Genesis hash: "
              << genesis.hash()
              << "\n";
    std::cout << "Genesis nonce: "
              << genesis.header().nonce
              << "\n";
    std::cout << "PoW difficulty: "
              << blockchain.difficulty()
              << "\n";
    std::cout << "Chain height: "
              << blockchain.size() - 1
              << "\n";
    std::cout << "Genesis validation: OK\n";
    std::cout << "=== LARB MAINNET READY ===\n";

    return 0;
}
