#include "src/crypto/pqc.h"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB PQC TEST ===" << std::endl;

    const std::string message =
        "LARB ML-DSA-44 signature test";

    const larb::PQCKeyPair keys =
        larb::PQC::generate_keypair();

    assert(
        keys.public_key.size() == 1312
    );

    assert(
        keys.private_key.size() == 2560
    );

    std::cout << "Keypair generation: OK"
              << std::endl;

    const std::string signature =
        larb::PQC::sign(
            message,
            keys.private_key
        );

    assert(
        signature.size() == 2420
    );

    std::cout << "Signing: OK"
              << std::endl;

    assert(
        larb::PQC::verify(
            message,
            signature,
            keys.public_key
        )
    );

    std::cout << "Verification: OK"
              << std::endl;

    assert(
        !larb::PQC::verify(
            message + "x",
            signature,
            keys.public_key
        )
    );

    std::cout << "Tampered message rejected: OK"
              << std::endl;

    std::cout << "=== ALL PQC TESTS PASSED ==="
              << std::endl;

    return 0;
}
