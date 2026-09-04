#include "src/genesis.h"
#include "src/script.h"
#include "src/transaction.h"
#include "src/consensus/pow.h"
#include "src/consensus/constants.h"

#include <iostream>

int main() {
    std::cout << "=== LARB CANONICAL GENESIS TEST ===\n";

    const larb::Block genesis = larb::Genesis::create();

    if (genesis.header().version != 1) {
        std::cerr << "Version: FAIL\n";
        return 1;
    }

    if (genesis.header().previous_hash != "0") {
        std::cerr << "Previous hash: FAIL\n";
        return 1;
    }

    if (genesis.header().timestamp !=
        larb::Genesis::TIMESTAMP) {
        std::cerr << "Timestamp: FAIL\n";
        return 1;
    }

    if (genesis.header().nonce !=
        larb::Genesis::NONCE) {
        std::cerr << "Nonce: FAIL\n";
        return 1;
    }

    if (genesis.transactions().size() != 1) {
        std::cerr << "Transaction count: FAIL\n";
        return 1;
    }

    const larb::Transaction tx =
        larb::Transaction::deserialize(
            genesis.transactions()[0]
        );

    if (tx.outputs.size() != 1) {
        std::cerr << "Output count: FAIL\n";
        return 1;
    }

    const std::string message =
        larb::Script::extract_op_return_data(
            tx.outputs[0].script_pubkey
        );

    if (message != larb::Genesis::MESSAGE) {
        std::cerr << "Genesis message: FAIL\n";
        return 1;
    }

    if (genesis.header().merkle_root !=
        genesis.calculate_merkle_root()) {
        std::cerr << "Merkle root: FAIL\n";
        return 1;
    }

    if (!larb::validate_proof_of_work(
            genesis,
            larb::INITIAL_POW_DIFFICULTY)) {
        std::cerr << "PoW: FAIL\n";
        return 1;
    }

    const std::string expected_hash =
        "00032373b4b4b78d9f32723dc913bec6c6eb3a0e2a5db92a16bb0f3be4ed909a";

    if (genesis.hash() != expected_hash) {
        std::cerr << "Genesis hash: FAIL\n";
        std::cerr << "Actual:   " << genesis.hash() << "\n";
        std::cerr << "Expected: " << expected_hash << "\n";
        return 1;
    }

    std::cout << "Version: OK\n";
    std::cout << "Previous hash: OK\n";
    std::cout << "Timestamp: OK\n";
    std::cout << "Nonce: OK\n";
    std::cout << "Transaction: OK\n";
    std::cout << "OP_RETURN: OK\n";
    std::cout << "Message: [" << message << "]\n";
    std::cout << "Merkle root: OK\n";
    std::cout << "PoW: OK\n";
    std::cout << "Canonical hash: OK\n";
    std::cout << "Genesis hash: " << genesis.hash() << "\n";

    std::cout << "=== CANONICAL GENESIS TEST PASSED ===\n";

    return 0;
}
