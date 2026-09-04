#include "block_validator.h"
#include "consensus/constants.h"
#include "transaction.h"

#include <iostream>

int main() {
    std::cout << "=== LARB DOUBLE SPEND BLOCK TEST ===\n";

    larb::UTXOSet utxos;

    /*
     * Buat UTXO awal secara manual.
     */
    const std::string funding_txid =
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

    utxos.add(
        larb::UTXO{
            larb::OutPoint{
                funding_txid,
                0
            },
            larb::TransactionOutput{
                1000,
                "OWNER"
            }
        }
    );

    /*
     * TX1 dan TX2 sama-sama mencoba
     * membelanjakan UTXO yang sama.
     */
    larb::Transaction tx1{};
    tx1.version = 1;

    tx1.inputs.push_back(
        larb::TransactionInput{
            funding_txid,
            0
        }
    );

    tx1.outputs.push_back(
        larb::TransactionOutput{
            900,
            "A"
        }
    );

    larb::Transaction tx2{};
    tx2.version = 1;

    tx2.inputs.push_back(
        larb::TransactionInput{
            funding_txid,
            0
        }
    );

    tx2.outputs.push_back(
        larb::TransactionOutput{
            800,
            "B"
        }
    );

    /*
     * Coinbase block.
     */
    const std::int64_t reward =
        larb::get_block_reward(1);

    larb::Transaction coinbase =
        larb::Transaction::coinbase(
            reward,
            "MINER"
        );

    larb::Block block(
        1,
        "previous",
        {
            coinbase.serialize(),
            tx1.serialize(),
            tx2.serialize()
        },
        1,
        0
    );

    /*
     * Snapshot sebelum validation.
     */
    const std::size_t original_size =
        utxos.size();

    /*
     * Block harus ditolak karena TX2
     * mencoba spend UTXO yang sudah dipakai TX1.
     */
    if (larb::validate_block(
            block,
            1,
            utxos)) {

        std::cerr
            << "Double-spend rejection: FAIL\n";

        return 1;
    }

    std::cout
        << "Double-spend rejection: OK\n";

    /*
     * Atomicity:
     * UTXO asli harus tetap utuh.
     */
    if (utxos.size() != original_size) {
        std::cerr
            << "UTXO rollback: FAIL\n";

        return 1;
    }

    if (!utxos.exists({
            funding_txid,
            0
        })) {

        std::cerr
            << "Original UTXO preserved: FAIL\n";

        return 1;
    }

    std::cout
        << "UTXO rollback: OK\n";

    std::cout
        << "Original UTXO preserved: OK\n";

    std::cout
        << "=== ALL DOUBLE SPEND TESTS PASSED ===\n";

    return 0;
}
