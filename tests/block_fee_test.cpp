#include "block.h"
#include "block_validator.h"
#include "consensus/constants.h"
#include "transaction.h"
#include "utxo_set.h"
#include "script.h"
#include "crypto/pqc.h"

#include <iostream>
#include <string>

int main() {
    using namespace larb;

    std::cout << "=== LARB BLOCK FEE TEST ===\n";

    auto keys = PQC::generate_keypair();

    UTXOSet utxos;

    Transaction funding;
    funding.version = 1;

    funding.outputs.push_back(
        TransactionOutput{
            1000,
            Script::pay_to_pubkey_hash(keys.public_key)
        }
    );

    const std::string funding_txid =
        funding.txid();

    utxos.add(UTXO{
        OutPoint{funding_txid, 0},
        funding.outputs[0]
    });

    const std::int64_t reward =
        get_block_reward(1);

    Transaction tx;
    tx.version = 1;

    tx.inputs.push_back(
        TransactionInput{
            funding_txid,
            0,
            "",
            ""
        }
    );

    tx.outputs.push_back(
        TransactionOutput{
            900,
            "recipient"
        }
    );

    if (!tx.sign_input(
            0,
            keys.public_key,
            keys.private_key)) {
        std::cerr << "Transaction signing: FAIL\n";
        return 1;
    }

    /*
     * Input = 1000
     * Output = 900
     * Fee = 100
     *
     * Coinbase = reward + 100
     */
    Transaction coinbase =
        Transaction::coinbase(
            reward + 100,
            "miner",
            1
        );

    Block block(
        1,
        "",
        {
            coinbase.serialize(),
            tx.serialize()
        },
        1,
        0
    );

    if (!validate_block(
            block,
            1,
            utxos)) {
        std::cout
            << "Reward + fee accepted: FAIL\n";
        return 1;
    }

    std::cout
        << "Reward + fee accepted: OK\n";

    /*
     * Overpay 1 satoshi.
     */
    UTXOSet overpay_utxos;

    overpay_utxos.add(UTXO{
        OutPoint{funding_txid, 0},
        funding.outputs[0]
    });

    Transaction overpay_coinbase =
        Transaction::coinbase(
            reward + 101,
            "miner",
            1
        );

    Block overpay_block(
        1,
        "",
        {
            overpay_coinbase.serialize(),
            tx.serialize()
        },
        1,
        0
    );

    if (validate_block(
            overpay_block,
            1,
            overpay_utxos)) {
        std::cout
            << "Overpay rejection: FAIL\n";
        return 1;
    }

    std::cout
        << "Overpay rejection: OK\n";

    std::cout
        << "=== ALL BLOCK FEE TESTS PASSED ===\n";

    return 0;
}
