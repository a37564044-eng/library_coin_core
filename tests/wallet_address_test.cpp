#include "src/wallet/wallet.h"

#include <iostream>

int main() {
    std::cout << "=== LARB WALLET ADDRESS TEST ===\n";

    larb::Wallet wallet_a;
    larb::Wallet wallet_b;

    const std::string address_a = wallet_a.address();
    const std::string address_b = wallet_b.address();

    std::cout << "Address A: " << address_a << "\n";
    std::cout << "Address B: " << address_b << "\n";

    if (address_a.empty() || address_b.empty()) {
        std::cerr << "Address generation: FAIL\n";
        return 1;
    }

    if (address_a == address_b) {
        std::cerr << "Address uniqueness: FAIL\n";
        return 1;
    }

    if (address_a.rfind("larb1q", 0) != 0 ||
        address_b.rfind("larb1q", 0) != 0) {
        std::cerr << "Address prefix: FAIL\n";
        return 1;
    }

    std::cout << "Address generation: OK\n";
    std::cout << "Address uniqueness: OK\n";
    std::cout << "Address prefix: OK\n";
    std::cout << "=== WALLET ADDRESS TEST PASSED ===\n";

    return 0;
}
