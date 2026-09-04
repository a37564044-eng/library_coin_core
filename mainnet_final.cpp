#include "src/mainnet_final.h"

#include <iostream>

int main() {
    std::cout << "=== LARB MAINNET FINAL CHECK ===\n";
    std::cout << "Network: "
              << larb::MainnetFinal::NETWORK
              << '\n';

    if (!larb::MainnetFinal::verify()) {
        std::cout << "Mainnet final verification: FAILED\n";
        return 1;
    }

    std::cout << "Genesis hash: "
              << larb::MainnetFinal::GENESIS_HASH
              << '\n';

    std::cout << "Genesis nonce: "
              << larb::MainnetFinal::GENESIS_NONCE
              << '\n';

    std::cout << "PoW difficulty: "
              << larb::MainnetFinal::POW_DIFFICULTY
              << '\n';

    std::cout << "Mainnet final verification: OK\n";
    std::cout << "=== LARB MAINNET FINALIZED ===\n";

    return 0;
}
