#include "src/block.h"
#include "src/blockchain.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static std::string serialize_block(const larb::Block& block) {
    const auto& header = block.header();

    std::ostringstream out;

    out << header.version << '\n';
    out << header.previous_hash << '\n';
    out << header.merkle_root << '\n';
    out << header.timestamp << '\n';
    out << header.nonce << '\n';

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
        const std::string block_data = serialize_block(chain.at(i));

        out << block_data.size() << '\n';
        out << block_data;
    }

    return out.str();
}

int main() {
    std::cout << "=== LARB CHAIN SERIALIZATION TEST ===\n";

    larb::Block genesis(
        1,
        "",
        {"LARB Genesis Block"},
        1700000000,
        0
    );

    larb::Blockchain chain(genesis, 0);

    larb::Block block1(
        1,
        genesis.hash(),
        {"transaction-1"},
        1700000001,
        1
    );

    larb::Block block2(
        1,
        block1.hash(),
        {"transaction-2"},
        1700000002,
        2
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

    const std::string serialized = serialize_chain(chain);

    if (serialized.empty()) {
        std::cerr << "Serialization: FAIL\n";
        return 1;
    }

    std::cout << "Serialization: OK\n";

    const std::string serialized_again = serialize_chain(chain);

    if (serialized != serialized_again) {
        std::cerr << "Deterministic serialization: FAIL\n";
        return 1;
    }

    std::cout << "Deterministic serialization: OK\n";

    if (serialized.find("LARB Genesis Block") == std::string::npos) {
        std::cerr << "Genesis data: FAIL\n";
        return 1;
    }

    if (serialized.find("transaction-1") == std::string::npos ||
        serialized.find("transaction-2") == std::string::npos) {
        std::cerr << "Transaction data: FAIL\n";
        return 1;
    }

    std::cout << "Transaction data: OK\n";

    std::cout << "Serialized chain size: "
              << serialized.size()
              << " bytes\n";

    std::cout << "=== ALL CHAIN SERIALIZATION TESTS PASSED ===\n";

    return 0;
}
