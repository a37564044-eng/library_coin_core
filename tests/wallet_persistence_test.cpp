#include "src/wallet/wallet.h"

#include <cstdio>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB WALLET PERSISTENCE TEST ===\n";

    const std::string path =
        "/tmp/larb_wallet_test.dat";

    std::remove(path.c_str());

    larb::Wallet original;

    const std::string original_address =
        original.address();

    const std::string message =
        "LARB WALLET PERSISTENCE PROOF";

    const std::string original_signature =
        original.sign(message);

    if (!original.verify(
            message,
            original_signature)) {
        std::cerr << "Original signing: FAIL\n";
        return 1;
    }

    std::cout << "Original wallet: OK\n";

    if (!original.save(path, "LARB-persistence-test-2026")) {
        std::cerr << "Wallet save: FAIL\n";
        return 1;
    }

    std::cout << "Wallet save: OK\n";

    larb::Wallet restored =
        larb::Wallet::load(path, "LARB-persistence-test-2026");

    std::cout << "Wallet load: OK\n";

    if (restored.address() !=
        original_address) {
        std::cerr << "Address preservation: FAIL\n";
        return 1;
    }

    std::cout << "Address preservation: OK\n";

    if (restored.public_key() !=
        original.public_key()) {
        std::cerr << "Public key preservation: FAIL\n";
        return 1;
    }

    std::cout << "Public key preservation: OK\n";

    if (restored.secret_key() !=
        original.secret_key()) {
        std::cerr << "Secret key preservation: FAIL\n";
        return 1;
    }

    std::cout << "Secret key preservation: OK\n";

    const std::string restored_signature =
        restored.sign(message);

    if (!restored.verify(
            message,
            restored_signature)) {
        std::cerr << "Restored signing: FAIL\n";
        return 1;
    }

    std::cout << "Restored signing: OK\n";

    if (!original.verify(
            message,
            restored_signature)) {
        std::cerr << "Cross-wallet verification: FAIL\n";
        return 1;
    }

    std::cout << "Cross-wallet verification: OK\n";

    std::remove(path.c_str());

    std::cout << "Address: "
              << original_address
              << "\n";

    std::cout << "=== WALLET PERSISTENCE TEST PASSED ===\n";

    return 0;
}
