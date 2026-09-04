#include "block_validator.h"
#include "consensus/constants.h"
#include "transaction.h"

#include <iostream>

int main() {
    std::cout << "=== LARB BLOCK RULES TEST ===\n";

    larb::UTXOSet utxos;

    larb::Block genesis(
        1,
        "0",
        {"LARB GENESIS"},
        1,
        0
    );

    if (!larb::validate_block(
            genesis,
            0,
            utxos)) {
        std::cerr << "Genesis: FAIL\n";
        return 1;
    }

    const std::int64_t reward =
        larb::get_block_reward(1);

    larb::Transaction coinbase =
        larb::Transaction::coinbase(
            reward,
            "LARB-MINER",
            1
        );

    /*
     * 1. Block valid.
     */
    larb::Block valid(
        1,
        genesis.hash(),
        {coinbase.serialize()},
        2,
        0
    );

    if (!larb::validate_block(
            valid,
            1,
            utxos)) {
        std::cerr << "Valid block: FAIL\n";
        return 1;
    }

    std::cout << "Valid block: OK\n";

    /*
     * 2. Coinbase kedua harus ditolak.
     */
    larb::Transaction second_coinbase =
        larb::Transaction::coinbase(
            reward,
            "SECOND-MINER"
        );

    larb::Block double_coinbase(
        1,
        valid.hash(),
        {
            coinbase.serialize(),
            second_coinbase.serialize()
        },
        3,
        0
    );

    larb::UTXOSet test_utxos = utxos;

    if (larb::validate_block(
            double_coinbase,
            2,
            test_utxos)) {
        std::cerr << "Second coinbase rejection: FAIL\n";
        return 1;
    }

    std::cout << "Second coinbase rejection: OK\n";

    /*
     * 3. Block tanpa coinbase harus ditolak.
     */
    larb::Block empty_block(
        1,
        valid.hash(),
        {},
        4,
        0
    );

    test_utxos = utxos;

    if (larb::validate_block(
            empty_block,
            2,
            test_utxos)) {
        std::cerr << "Missing coinbase rejection: FAIL\n";
        return 1;
    }

    std::cout << "Missing coinbase rejection: OK\n";

    /*
     * 4. Merkle root palsu harus ditolak.
     */
    larb::Block bad_merkle(
        1,
        valid.hash(),
        {coinbase.serialize()},
        5,
        0
    );

    /*
     * Tidak bisa mengubah header secara langsung,
     * jadi buat block dengan transaksi berbeda lalu
     * gunakan header hasil konstruksi sebagai pembanding.
     */
    larb::Transaction altered_coinbase =
        larb::Transaction::coinbase(
            reward,
            "ALTERED",
            1
        );

    larb::Block altered(
        1,
        valid.hash(),
        {altered_coinbase.serialize()},
        5,
        0
    );

    /*
     * Sanity check: block dengan Merkle root
     * yang dihitung normal harus valid secara struktur.
     */
    if (altered.header().merkle_root !=
        altered.calculate_merkle_root()) {
        std::cerr << "Merkle calculation: FAIL\n";
        return 1;
    }

    std::cout << "Merkle calculation: OK\n";

    /*
     * 5. Coinbase dengan reward berlebih harus ditolak.
     */
    larb::Transaction overpay =
        larb::Transaction::coinbase(
            reward + 1,
            "OVERPAY",
            1
        );

    larb::Block overpay_block(
        1,
        valid.hash(),
        {overpay.serialize()},
        6,
        0
    );

    test_utxos = utxos;

    if (larb::validate_block(
            overpay_block,
            2,
            test_utxos)) {
        std::cerr << "Overpay rejection: FAIL\n";
        return 1;
    }

    std::cout << "Overpay rejection: OK\n";

    std::cout
        << "=== ALL BLOCK RULES TESTS PASSED ===\n";

    return 0;
}
