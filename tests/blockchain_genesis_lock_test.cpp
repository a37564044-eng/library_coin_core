#include "src/blockchain.h"
#include "src/genesis.h"
#include "src/consensus/pow.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB BLOCKCHAIN GENESIS LOCK TEST ===\n";

    // Canonical Genesis
    const larb::Block genesis =
        larb::Genesis::create();

    larb::Blockchain chain(genesis);

    if (chain.size() != 1) {
        std::cerr << "Canonical genesis size: FAIL\n";
        return 1;
    }

    std::cout << "Canonical genesis accepted: OK\n";

    if (!chain.is_valid()) {
        std::cerr << "Canonical chain valid: FAIL\n";
        return 1;
    }

    std::cout << "Canonical chain valid: OK\n";

    // Buat block palsu secara manual:
    // transaksi sama, tetapi timestamp berbeda.
    const larb::Block fake_genesis(
        genesis.header().version,
        "0",
        genesis.transactions(),
        genesis.header().timestamp + 1,
        genesis.header().nonce
    );

    // Hash harus berbeda.
    if (fake_genesis.hash() ==
        genesis.hash()) {
        std::cerr << "Fake genesis hash differs: FAIL\n";
        return 1;
    }

    std::cout << "Fake genesis hash differs: OK\n";

    // Blockchain palsu dibuat, tetapi harus dianggap invalid.
    larb::Blockchain fake_chain(fake_genesis);

    if (fake_chain.is_valid()) {
        std::cerr << "Fake genesis rejection: FAIL\n";
        return 1;
    }

    std::cout << "Fake genesis rejection: OK\n";

    // Pastikan hash canonical tetap terkunci.
    const std::string expected_hash =
        "00032373b4b4b78d9f32723dc913bec6c6eb3a0e2a5db92a16bb0f3be4ed909a";

    if (genesis.hash() != expected_hash) {
        std::cerr << "Genesis hash lock: FAIL\n";
        std::cerr << "Actual: "
                  << genesis.hash() << "\n";
        return 1;
    }

    std::cout << "Genesis hash lock: OK\n";
    std::cout << "Genesis hash: "
              << genesis.hash()
              << "\n";

    std::cout
        << "=== BLOCKCHAIN GENESIS LOCK TEST PASSED ===\n";

    return 0;
}
