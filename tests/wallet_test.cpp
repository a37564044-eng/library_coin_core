#include "src/wallet/wallet.h"

#include <cassert>
#include <iostream>

int main() {
    larb::Wallet wallet_a;
    larb::Wallet wallet_b;

    assert(
        wallet_a.public_key().size() ==
        larb::Wallet::PUBLIC_KEY_SIZE
    );

    assert(
        wallet_a.secret_key().size() ==
        larb::Wallet::SECRET_KEY_SIZE
    );

    assert(
        wallet_a.public_key() !=
        wallet_b.public_key()
    );

    assert(
        wallet_a.secret_key() !=
        wallet_b.secret_key()
    );

    const std::string message =
        "LARB wallet signing test";

    const std::string signature =
        wallet_a.sign(message);

    assert(
        signature.size() ==
        larb::Wallet::SIGNATURE_SIZE
    );

    assert(wallet_a.verify(message, signature));

    assert(
        !wallet_a.verify(
            "tampered",
            signature
        )
    );

    assert(
        !wallet_b.verify(
            message,
            signature
        )
    );

    std::cout
        << "=== LARB Wallet Test ===\n"
        << "Key generation: OK\n"
        << "Key sizes: OK\n"
        << "Independent keypairs: OK\n"
        << "Signing: OK\n"
        << "Verification: OK\n"
        << "Tampered message rejected: OK\n"
        << "Wrong wallet rejected: OK\n"
        << "=== ALL WALLET TESTS PASSED ===\n";

    return 0;
}
