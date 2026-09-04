#include "src/wallet/wallet.h"
#include "src/crypto/pqc.h"
#include "src/consensus/constants.h"

#include <iostream>
#include <string>

int main() {
    larb::Wallet wallet;
    std::cout << "Mining address: " << wallet.address() << std::endl;


    // ==============================
    // LARB ML-DSA-44 TEST
    // ==============================

    std::cout << "=== LARB ML-DSA-44 Test ===" << std::endl;

    const auto keypair = larb::PQC::generate_keypair();

    const std::string message = "LARB transaction test";

    const std::string signature =
        larb::PQC::sign(message, keypair.private_key);

    const bool verified =
        larb::PQC::verify(
            message,
            signature,
            keypair.public_key
        );

    std::cout << "ML-DSA-44 keypair: "
              << (!keypair.public_key.empty() &&
                  !keypair.private_key.empty()
                      ? "OK"
                      : "FAILED")
              << std::endl;

    std::cout << "ML-DSA-44 sign: "
              << (!signature.empty()
                      ? "OK"
                      : "FAILED")
              << std::endl;

    std::cout << "ML-DSA-44 verify: "
              << (verified
                      ? "OK"
                      : "FAILED")
              << std::endl;


    // ==============================
    // TAMPER TEST
    // ==============================

    const std::string tampered_message =
        "LARB transaction tampered";

    const bool tampered =
        larb::PQC::verify(
            tampered_message,
            signature,
            keypair.public_key
        );

    std::cout << "Tampered message rejected: "
              << (!tampered
                      ? "OK"
                      : "FAILED")
              << std::endl;


    // ==============================
    // LARB HALVING TEST
    // ==============================

    std::cout << "=== LARB Halving Test ===" << std::endl;

    std::cout << "Block 0 reward: "
              << larb::get_block_reward(0)
              << " satoshis"
              << std::endl;

    std::cout << "Block 210000 reward: "
              << larb::get_block_reward(210000)
              << " satoshis"
              << std::endl;

    std::cout << "Block 420000 reward: "
              << larb::get_block_reward(420000)
              << " satoshis"
              << std::endl;

    std::cout << "Block 630000 reward: "
              << larb::get_block_reward(630000)
              << " satoshis"
              << std::endl;

    std::cout << "Block 840000 reward: "
              << larb::get_block_reward(840000)
              << " satoshis"
              << std::endl;

    std::cout << "Block 1260000 reward: "
              << larb::get_block_reward(1260000)
              << " satoshis"
              << std::endl;


    // ==============================
    // FINAL RESULT
    // ==============================

    return (verified && !tampered) ? 0 : 1;
}
