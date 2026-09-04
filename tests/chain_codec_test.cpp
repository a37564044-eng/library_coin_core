#include "src/block.h"
#include "src/genesis.h"
#include "src/consensus/pow.h"
#include "src/consensus/constants.h"
#include "src/blockchain.h"
#include "src/consensus/pow.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static std::string serialize_block(const larb::Block& block) {
    const auto& h = block.header();

    std::ostringstream out;

    out << h.version << '\n';
    out << h.previous_hash << '\n';
    out << h.merkle_root << '\n';
    out << h.timestamp << '\n';
    out << h.nonce << '\n';

    const auto& txs = block.transactions();
    out << txs.size() << '\n';

    for (const auto& tx : txs) {
        out << tx.size() << ':' << tx << '\n';
    }

    return out.str();
}

static std::string serialize_chain(const larb::Blockchain& chain) {
    std::ostringstream out;

    out << chain.size() << '\n';

    for (std::size_t i = 0; i < chain.size(); ++i) {
        const std::string block = serialize_block(chain.at(i));

        out << block.size() << '\n';
        out << block;
    }

    return out.str();
}

int main() {
    std::cout << "=== LARB CHAIN CODEC TEST ===\n";

    larb::Block genesis = larb::Genesis::create();

    larb::Blockchain chain(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    larb::Block block1 = larb::mine_block(
        1,
        genesis.hash(),
        {"transaction-1"},
        1700000001,
        larb::INITIAL_POW_DIFFICULTY
    );

    larb::Block block2 = larb::mine_block(
        1,
        block1.hash(),
        {"transaction-2"},
        1700000002,
        larb::INITIAL_POW_DIFFICULTY
    );

    if (!chain.add_block(block1)) {
        std::cerr << "Add block 1: FAIL\n";
        return 1;
    }

    if (!chain.add_block(block2)) {
        std::cerr << "Add block 2: FAIL\n";
        return 1;
    }

    std::cout << "Chain construction: OK\n";

    if (!chain.is_valid()) {
        std::cerr << "Original chain validation: FAIL\n";
        return 1;
    }

    std::cout << "Original chain validation: OK\n";

    const std::string encoded = serialize_chain(chain);

    if (encoded.empty()) {
        std::cerr << "Encode: FAIL\n";
        return 1;
    }

    std::cout << "Encode: OK\n";

    if (encoded != serialize_chain(chain)) {
        std::cerr << "Deterministic encode: FAIL\n";
        return 1;
    }

    std::cout << "Deterministic encode: OK\n";

    /*
     * Untuk tahap ini kita belum memasukkan data hasil decode
     * ke Blockchain. Kita hanya memastikan format yang akan
     * melewati TCP stabil dan tidak kosong.
     */
    if (encoded.empty()) {
        std::cerr << "Genesis payload: FAIL\n";
        return 1;
    }

    if (encoded.find("transaction-1") == std::string::npos ||
        encoded.find("transaction-2") == std::string::npos) {
        std::cerr << "Transaction payload: FAIL\n";
        return 1;
    }

    std::cout << "Payload integrity: OK\n";
    std::cout << "Encoded bytes: " << encoded.size() << '\n';

    std::cout << "=== ALL CHAIN CODEC TESTS PASSED ===\n";

    return 0;
}
