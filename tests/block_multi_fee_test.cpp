#include "block.h"
#include "block_validator.h"
#include "transaction_validator.h"
#include "consensus/constants.h"
#include "transaction.h"
#include "utxo_set.h"
#include "script.h"
#include "crypto/pqc.h"

#include <iostream>
#include <string>

int main() {
    using namespace larb;

    std::cout << "=== LARB MULTI TRANSACTION FEE TEST ===\n";

    auto keys1 = PQC::generate_keypair();
    auto keys2 = PQC::generate_keypair();

    UTXOSet utxos;

    /*
     * Funding TX 1
     */
    Transaction funding1;
    funding1.version = 1;

    funding1.outputs.push_back(
        TransactionOutput{
            1000,
            Script::pay_to_pubkey_hash(keys1.public_key)
        }
    );

    const std::string funding1_txid =
        funding1.txid();

    utxos.add(UTXO{
        OutPoint{funding1_txid, 0},
        funding1.outputs[0]
    });

    /*
     * Funding TX 2
     */
    Transaction funding2;
    funding2.version = 1;

    funding2.outputs.push_back(
        TransactionOutput{
            2000,
            Script::pay_to_pubkey_hash(keys2.public_key)
        }
    );

    const std::string funding2_txid =
        funding2.txid();

    utxos.add(UTXO{
        OutPoint{funding2_txid, 0},
        funding2.outputs[0]
    });

    /*
     * TX1:
     * input  = 1000
     * output = 900
     * fee    = 100
     */
    Transaction tx1;
    tx1.version = 1;

    tx1.inputs.push_back(
        TransactionInput{
            funding1_txid,
            0,
            "",
            ""
        }
    );

    tx1.outputs.push_back(
        TransactionOutput{
            900,
            "recipient1"
        }
    );

    if (!tx1.sign_input(
            0,
            keys1.public_key,
            keys1.private_key)) {
        std::cerr
            << "TX1 signing: FAIL\n";
        return 1;
    }

    /*
     * TX2:
     * input  = 2000
     * output = 1800
     * fee    = 200
     */
    Transaction tx2;
    tx2.version = 1;

    tx2.inputs.push_back(
        TransactionInput{
            funding2_txid,
            0,
            "",
            ""
        }
    );

    tx2.outputs.push_back(
        TransactionOutput{
            1800,
            "recipient2"
        }
    );

    if (!tx2.sign_input(
            0,
            keys2.public_key,
            keys2.private_key)) {
        std::cerr
            << "TX2 signing: FAIL\n";
        return 1;
    }

    /*
     * Pastikan kedua transaksi memang valid
     * sebelum masuk ke block.
     */
    if (!validate_transaction_inputs(tx1, utxos)) {
        std::cerr
            << "TX1 validation: FAIL\n";
        return 1;
    }

    if (!validate_transaction_inputs(tx2, utxos)) {
        std::cerr
            << "TX2 validation: FAIL\n";
        return 1;
    }

    const std::int64_t total_fee = 300;

    const std::int64_t reward =
        get_block_reward(1);

    /*
     * Coinbase = block reward + seluruh fee.
     */
    Transaction coinbase =
        Transaction::coinbase(
            reward + total_fee,
            "miner",
            1
        );

    Block block(
        1,
        "",
        {
            coinbase.serialize(),
            tx1.serialize(),
            tx2.serialize()
        },
        1,
        0
    );

    if (!validate_block(
            block,
            1,
            utxos)) {
        std::cout
            << "Two transaction fees accepted: FAIL\n";
        return 1;
    }

    std::cout
        << "TX1 fee = 100: OK\n";

    std::cout
        << "TX2 fee = 200: OK\n";

    std::cout
        << "Total fee = 300: OK\n";

    std::cout
        << "Reward + all fees accepted: OK\n";

    /*
     * Sekarang coba coinbase mengambil 1 satoshi terlalu banyak.
     */
    UTXOSet overpay_utxos;

    overpay_utxos.add(UTXO{
        OutPoint{funding1_txid, 0},
        funding1.outputs[0]
    });

    overpay_utxos.add(UTXO{
        OutPoint{funding2_txid, 0},
        funding2.outputs[0]
    });

    Transaction overpay_coinbase =
        Transaction::coinbase(
            reward + total_fee + 1,
            "miner",
            1
        );

    Block overpay_block(
        1,
        "",
        {
            overpay_coinbase.serialize(),
            tx1.serialize(),
            tx2.serialize()
        },
        1,
        0
    );

    if (validate_block(
            overpay_block,
            1,
            overpay_utxos)) {
        std::cout
            << "Multi-fee overpay rejection: FAIL\n";
        return 1;
    }

    std::cout
        << "Multi-fee overpay rejection: OK\n";

    std::cout
        << "=== ALL MULTI TRANSACTION FEE TESTS PASSED ===\n";

    return 0;
}
