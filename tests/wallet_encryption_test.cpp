#include "wallet/wallet.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    const std::string path = "wallet_encryption_test.dat";
    const std::string password = "LARB-test-password-2026";

    larb::Wallet original;

    const std::string original_address =
        original.address();

    assert(original.save(path, password));

    larb::Wallet recovered =
        larb::Wallet::load(path, password);

    assert(recovered.address() == original_address);

    const std::string message =
        "LARB WALLET RECOVERY TEST";

    const std::string signature =
        recovered.sign(message);

    assert(recovered.verify(message, signature));

    bool wrong_password_rejected = false;

    try {
        larb::Wallet::load(
            path,
            "wrong-password"
        );
    } catch (...) {
        wrong_password_rejected = true;
    }

    assert(wrong_password_rejected);

    {
        std::fstream file(
            path,
            std::ios::in |
            std::ios::out |
            std::ios::binary
        );

        assert(file.good());

        file.seekp(40);
        char byte = 0;
        file.read(&byte, 1);
        file.seekp(40);

        byte ^= static_cast<char>(0xff);
        file.write(&byte, 1);
    }

    bool corruption_rejected = false;

    try {
        larb::Wallet::load(path, password);
    } catch (...) {
        corruption_rejected = true;
    }

    assert(corruption_rejected);

    std::remove(path.c_str());

    std::cout
        << "Wallet encryption/recovery: PASS\n";

    return 0;
}
