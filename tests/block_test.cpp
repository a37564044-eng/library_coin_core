#include "block.h"

#include <iostream>
#include <string>
#include <vector>

int main() {
    std::cout << "=== LARB BLOCK TEST ===" << std::endl;

    const std::vector<std::string> transactions = {
        "TX-001",
        "TX-002",
        "TX-003"
    };

    larb::Block block(
        1,
        "0000000000000000000000000000000000000000000000000000000000000000",
        transactions,
        1700000000,
        0
    );

    bool tx_ok =
        block.transactions().size() == 3;

    std::cout
        << "Transaction count: "
        << (tx_ok ? "OK" : "FAILED")
        << std::endl;

    const std::string merkle =
        block.calculate_merkle_root();

    bool merkle_ok =
        !merkle.empty();

    std::cout
        << "Merkle root: "
        << (merkle_ok ? "OK" : "FAILED")
        << std::endl;

    const std::string hash1 =
        block.hash();

    const std::string hash2 =
        block.hash();

    bool deterministic =
        !hash1.empty() &&
        hash1 == hash2;

    std::cout
        << "Block hash deterministic: "
        << (deterministic ? "OK" : "FAILED")
        << std::endl;

    std::cout
        << "Block hash: "
        << hash1
        << std::endl;

    bool previous_ok =
        block.header().previous_hash ==
        "0000000000000000000000000000000000000000000000000000000000000000";

    std::cout
        << "Previous block hash: "
        << (previous_ok ? "OK" : "FAILED")
        << std::endl;

    const bool all_ok =
        tx_ok &&
        merkle_ok &&
        deterministic &&
        previous_ok;

    std::cout
        << (all_ok
            ? "=== ALL BLOCK TESTS PASSED ==="
            : "=== BLOCK TEST FAILED ===")
        << std::endl;

    return all_ok ? 0 : 1;
}
