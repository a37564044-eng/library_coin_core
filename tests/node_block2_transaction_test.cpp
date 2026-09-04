#include "src/genesis.h"
#include "src/node.h"
#include "src/transaction.h"
#include "src/utxo_set.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB NODE BLOCK #2 TRANSACTION TEST ===\n";

    const larb::Block genesis =
        larb::Genesis::create();

    larb::Node node(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    /*
     * BLOCK #1
     */
    const std::int64_t reward1 =
        larb::get_block_reward(1);

    const larb::Transaction coinbase1 =
        larb::Transaction::coinbase(
            reward1,
            "MINER-1"
        );

    const larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase1.serialize()},
            2,
            larb::INITIAL_POW_DIFFICULTY
        );

    if (!node.receive_block(block1)) {
        std::cerr << "Block #1: FAIL\n";
        return 1;
    }

    std::cout << "Block #1 accepted: OK\n";

    /*
     * Ambil TXID coinbase Block #1.
     */
    const std::string funding_txid =
        coinbase1.txid();

    const std::int64_t funding_amount =
        reward1;

    /*
     * UTXO yang akan dibelanjakan.
     */
    larb::OutPoint funding_outpoint{
        funding_txid,
        0
    };

    /*
     * Spend 50 LARB:
     *
     * 49 LARB -> receiver
     * 1 LARB  -> fee
     */
    const std::int64_t receiver_amount =
        funding_amount - 100'000'000;

    larb::Transaction tx;

    tx.version = 1;

    tx.inputs.push_back(
        larb::TransactionInput{
            funding_txid,
            0
        }
    );

    tx.outputs.push_back(
        larb::TransactionOutput{
            receiver_amount,
            "receiver"
        }
    );

    const larb::Transaction coinbase2 =
        larb::Transaction::coinbase(
            larb::get_block_reward(2) + 100'000'000,
            "MINER-2"
        );

    const larb::Block block2 =
        larb::mine_block(
            1,
            block1.hash(),
            {
                coinbase2.serialize(),
                tx.serialize()
            },
            3,
            larb::INITIAL_POW_DIFFICULTY
        );

    if (!node.receive_block(block2)) {
        std::cerr << "Block #2 transaction: FAIL\n";
        return 1;
    }

    std::cout << "Block #2 accepted: OK\n";
    std::cout << "Transaction spend: OK\n";
    std::cout << "Fee: 100000000 satoshi: OK\n";
    std::cout << "Chain size = "
              << node.chain_size()
              << ": "
              << (node.chain_size() == 3 ? "OK" : "FAIL")
              << "\n";

    if (node.chain_size() != 3) {
        return 1;
    }

    if (!node.is_chain_valid()) {
        std::cerr << "Final chain validity: FAIL\n";
        return 1;
    }

    std::cout << "Final chain validity: OK\n";
    std::cout << "Block #2 hash: "
              << block2.hash()
              << "\n";
    std::cout << "Block #2 nonce: "
              << block2.header().nonce
              << "\n";

    std::cout << "=== NODE BLOCK #2 TRANSACTION TEST PASSED ===\n";

    return 0;
}
