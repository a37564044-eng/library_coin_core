#include "block_validator.h"
#include "consensus/constants.h"
#include "transaction.h"

#include <iostream>

int main() {
    std::cout << "=== LARB BLOCK VALIDATOR TEST ===\n";

    larb::UTXOSet utxos;

    // Genesis adalah artefak dan tidak menghasilkan UTXO.
    larb::Block genesis(
        1,
        "0",
        {"LARB GENESIS"},
        1,
        0
    );

    if (!larb::validate_block(genesis, 0, utxos)) {
        std::cerr << "Genesis validation: FAIL\n";
        return 1;
    }

    if (utxos.size() != 0) {
        std::cerr << "Genesis UTXO: FAIL\n";
        return 1;
    }

    std::cout << "Genesis artefact: OK\n";
    std::cout << "Genesis UTXO excluded: OK\n";

    // Block 1: coinbase menghasilkan spendable UTXO.
    const std::int64_t reward =
        larb::get_block_reward(1);

    larb::Transaction coinbase =
        larb::Transaction::coinbase(
            reward,
            "LARB-MINER",
            1
        );

    larb::Block block1(
        1,
        genesis.hash(),
        {coinbase.serialize()},
        2,
        0
    );

    if (!larb::validate_block(
            block1,
            1,
            utxos)) {
        std::cerr << "Block 1 validation: FAIL\n";
        return 1;
    }

    if (utxos.size() != 1) {
        std::cerr << "Block 1 UTXO: FAIL\n";
        return 1;
    }

    const larb::UTXO* reward_utxo =
        utxos.find({
            coinbase.txid(),
            0
        });

    if (reward_utxo == nullptr) {
        std::cerr << "Coinbase UTXO: FAIL\n";
        return 1;
    }

    if (reward_utxo->output.amount != reward) {
        std::cerr << "Coinbase reward: FAIL\n";
        return 1;
    }

    std::cout << "Block 1 coinbase: OK\n";
    std::cout << "Block 1 reward UTXO: OK\n";

    // Halving boundary.
    larb::UTXOSet halving_utxos;

    const std::uint64_t halving_height =
        larb::HALVING_INTERVAL;

    const std::int64_t halving_reward =
        larb::get_block_reward(halving_height);

    larb::Transaction halving_coinbase =
        larb::Transaction::coinbase(
            halving_reward,
            "LARB-HALVING",
            halving_height
        );

    larb::Block halving_block(
        1,
        "previous",
        {halving_coinbase.serialize()},
        3,
        0
    );

    if (!larb::validate_block(
            halving_block,
            halving_height,
            halving_utxos)) {
        std::cerr << "Halving block validation: FAIL\n";
        return 1;
    }

    std::cout << "Halving reward: OK\n";

    std::cout << "=== ALL BLOCK VALIDATOR TESTS PASSED ===\n";
    return 0;
}
