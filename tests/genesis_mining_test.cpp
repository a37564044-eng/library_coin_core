#include "src/genesis.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"
#include "src/script.h"
#include "src/transaction.h"

#include <iostream>
#include <string>
#include <vector>

int main() {
    std::cout << "=== LARB GENESIS MINING TEST ===\n";

    const std::string message =
        "LARB GENESIS - FREEDOM OF INFORMATION";

    const std::string script =
        larb::Script::op_return(message);

    larb::Transaction tx{};
    tx.version = 1;

    tx.outputs.push_back(
        larb::TransactionOutput::op_return(message)
    );

    const std::string serialized_tx =
        tx.serialize();

    const std::vector<std::string> transactions = {
        serialized_tx
    };

    const std::uint64_t timestamp = 1;

    const larb::Block genesis =
        larb::mine_block(
            1,
            "0",
            transactions,
            timestamp,
            larb::INITIAL_POW_DIFFICULTY
        );

    const auto& header = genesis.header();

    if (header.previous_hash != "0") {
        std::cerr << "Previous hash: FAIL\n";
        return 1;
    }

    std::cout << "Previous hash: OK\n";

    if (header.merkle_root !=
        genesis.calculate_merkle_root()) {
        std::cerr << "Merkle root: FAIL\n";
        return 1;
    }

    std::cout << "Merkle root: OK\n";

    if (!larb::validate_proof_of_work(
            genesis,
            larb::INITIAL_POW_DIFFICULTY)) {
        std::cerr << "PoW: FAIL\n";
        return 1;
    }

    std::cout << "PoW: OK\n";

    if (genesis.hash().substr(
            0,
            larb::INITIAL_POW_DIFFICULTY
        ) != "000") {
        std::cerr << "Difficulty prefix: FAIL\n";
        return 1;
    }

    std::cout << "Difficulty prefix: OK\n";

    const larb::Transaction decoded =
        larb::Transaction::deserialize(
            genesis.transactions()[0]
        );

    if (decoded.outputs.size() != 1) {
        std::cerr << "Genesis transaction: FAIL\n";
        return 1;
    }

    const std::string recovered =
        larb::Script::extract_op_return_data(
            decoded.outputs[0].script_pubkey
        );

    if (recovered != message) {
        std::cerr << "Genesis message: FAIL\n";
        return 1;
    }

    std::cout << "Genesis transaction: OK\n";
    std::cout << "OP_RETURN: OK\n";
    std::cout << "Genesis message: ["
              << recovered
              << "]\n";

    std::cout << "Nonce: "
              << header.nonce
              << "\n";

    std::cout << "Merkle root: "
              << header.merkle_root
              << "\n";

    std::cout << "Genesis hash: "
              << genesis.hash()
              << "\n";

    std::cout << "=== GENESIS MINING TEST PASSED ===\n";

    return 0;
}
