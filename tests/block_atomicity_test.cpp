#include "block_validator.h"
#include "consensus/constants.h"
#include "transaction.h"

#include <iostream>

int main() {
    std::cout << "=== LARB BLOCK ATOMICITY TEST ===\n";

    larb::UTXOSet utxos;

    // Genesis tidak menghasilkan UTXO.
    larb::Block genesis(
        1,
        "0",
        {"LARB GENESIS"},
        1,
        0
    );

    if (!larb::validate_block(genesis, 0, utxos)) {
        std::cerr << "Genesis: FAIL\n";
        return 1;
    }

    // Block 1 menghasilkan coinbase.
    larb::Transaction coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "MINER",
            1
        );

    larb::Block block1(
        1,
        genesis.hash(),
        {coinbase.serialize()},
        2,
        0
    );

    if (!larb::validate_block(block1, 1, utxos)) {
        std::cerr << "Block 1: FAIL\n";
        return 1;
    }

    const std::size_t before =
        utxos.size();

    if (before != 1) {
        std::cerr << "Initial UTXO count: FAIL\n";
        return 1;
    }

    /*
     * Transaksi pertama valid:
     * membelanjakan coinbase Block 1.
     */
    larb::Transaction spend;
    spend.version = 1;

    spend.inputs.push_back({
        coinbase.txid(),
        0
    });

    spend.outputs.push_back({
        larb::get_block_reward(1),
        "ALICE"
    });

    /*
     * Transaksi kedua sengaja invalid:
     * mencoba membelanjakan output yang sama
     * untuk kedua kalinya.
     */
    larb::Transaction double_spend;
    double_spend.version = 1;

    double_spend.inputs.push_back({
        coinbase.txid(),
        0
    });

    double_spend.outputs.push_back({
        1,
        "ATTACKER"
    });

    larb::Block bad_block(
        1,
        block1.hash(),
        {
            // Coinbase block 2
            larb::Transaction::coinbase(
                larb::get_block_reward(2),
                "MINER",
                2
            ).serialize(),

            // valid
            spend.serialize(),

            // invalid double spend
            double_spend.serialize()
        },
        3,
        0
    );

    if (larb::validate_block(
            bad_block,
            2,
            utxos)) {
        std::cerr << "Invalid block accepted: FAIL\n";
        return 1;
    }

    std::cout << "Invalid block rejected: OK\n";

    /*
     * Untuk desain atomic:
     * UTXO set asli harus tetap seperti sebelum
     * validasi block gagal.
     */
    if (utxos.size() != before) {
        std::cerr << "UTXO rollback: FAIL\n";
        return 1;
    }

    if (utxos.find({
            coinbase.txid(),
            0
        }) == nullptr) {
        std::cerr << "Original UTXO preserved: FAIL\n";
        return 1;
    }

    std::cout << "UTXO rollback: OK\n";
    std::cout << "Original UTXO preserved: OK\n";

    std::cout
        << "=== ALL BLOCK ATOMICITY TESTS PASSED ===\n";

    return 0;
}
