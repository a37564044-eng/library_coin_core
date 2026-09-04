#include "transaction_validator.h"
#include "utxo_set.h"

#include <iostream>

int main() {
    std::cout << "=== LARB DUPLICATE INPUT TEST ===\n";

    larb::UTXOSet utxos;

    const std::string txid =
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    utxos.add(
        larb::UTXO{
            larb::OutPoint{txid, 0},
            larb::TransactionOutput{
                1000,
                "OWNER"
            }
        }
    );

    larb::Transaction tx{};
    tx.version = 1;

    /*
     * Input yang sama sengaja digunakan dua kali.
     */
    tx.inputs.push_back(
        larb::TransactionInput{
            txid,
            0
        }
    );

    tx.inputs.push_back(
        larb::TransactionInput{
            txid,
            0
        }
    );

    tx.outputs.push_back(
        larb::TransactionOutput{
            1500,
            "DESTINATION"
        }
    );

    /*
     * Harus ditolak:
     * UTXO hanya bernilai 1000, bukan 2000.
     */
    if (larb::validate_transaction_value(
            tx,
            utxos)) {

        std::cerr
            << "Duplicate input rejection: FAIL\n";

        return 1;
    }

    std::cout
        << "Duplicate input rejection: OK\n";

    /*
     * UTXO asli harus tetap ada karena
     * transaksi tidak pernah diterapkan.
     */
    if (!utxos.exists({
            txid,
            0
        })) {

        std::cerr
            << "Original UTXO preserved: FAIL\n";

        return 1;
    }

    std::cout
        << "Original UTXO preserved: OK\n";

    std::cout
        << "=== ALL DUPLICATE INPUT TESTS PASSED ===\n";

    return 0;
}
