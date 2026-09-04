#include "node.h"
#include "genesis.h"
#include "transaction.h"
#include "consensus/constants.h"
#include "consensus/pow.h"

#include <cassert>
#include <iostream>

int main() {
    std::cout << "=== LARB FORK REORG UTXO TEST ===\n";

    constexpr std::uint32_t difficulty = 3;

    const larb::Block genesis = larb::Genesis::create();

    larb::Node node(genesis, difficulty);

    /*
     * =========================
     * FORK A
     * Genesis -> A1 -> A2
     * =========================
     */
    larb::Transaction a1_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "FORK-A-1",
            1
        );

    larb::Block a1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {a1_coinbase.serialize()},
            1700000001,
            difficulty
        );

    assert(node.receive_block(a1));

    larb::Transaction a2_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(2),
            "FORK-A-2",
            2
        );

    larb::Block a2 =
        larb::mine_block(
            1,
            a1.hash(),
            {a2_coinbase.serialize()},
            1700000002,
            difficulty
        );

    assert(node.receive_block(a2));

    const std::string a1_txid = a1_coinbase.txid();
    const std::string a2_txid = a2_coinbase.txid();

    assert(node.utxos().exists({a1_txid, 0}));
    assert(node.utxos().exists({a2_txid, 0}));

    std::cout << "Fork A accepted: OK\n";
    std::cout << "Fork A UTXO state: OK\n";

    /*
     * =========================
     * FORK B
     * Genesis -> B1 -> B2 -> B3
     * =========================
     */
    larb::Transaction b1_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "FORK-B-1",
            1
        );

    larb::Block b1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {b1_coinbase.serialize()},
            1700000011,
            difficulty
        );

    larb::Transaction b2_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(2),
            "FORK-B-2",
            2
        );

    larb::Block b2 =
        larb::mine_block(
            1,
            b1.hash(),
            {b2_coinbase.serialize()},
            1700000012,
            difficulty
        );

    larb::Transaction b3_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(3),
            "FORK-B-3",
            3
        );

    larb::Block b3 =
        larb::mine_block(
            1,
            b2.hash(),
            {b3_coinbase.serialize()},
            1700000013,
            difficulty
        );

    larb::Blockchain fork_b(genesis, difficulty);

    assert(fork_b.add_block(b1));
    assert(fork_b.add_block(b2));
    assert(fork_b.add_block(b3));

    std::cout << "Competing fork constructed: OK\n";

    /*
     * B memiliki work lebih tinggi:
     * A = 2 mined blocks
     * B = 3 mined blocks
     */
    assert(
        fork_b.chain_work() >
        node.blockchain().chain_work()
    );

    std::cout << "Higher chainwork detected: OK\n";

    /*
     * Simpan tip lama untuk memastikan berubah.
     */
    const std::string old_tip =
        node.blockchain()
            .at(node.chain_size() - 1)
            .hash();

    const std::string b3_txid =
        b3_coinbase.txid();

    /*
     * REORG
     */
    assert(node.adopt_chain(fork_b));

    std::cout << "Fork B adopted: OK\n";

    /*
     * Tip harus sekarang B3.
     */
    const std::string new_tip =
        node.blockchain()
            .at(node.chain_size() - 1)
            .hash();

    assert(new_tip == b3.hash());
    assert(new_tip != old_tip);

    std::cout << "Tip switched to winning fork: OK\n";

    /*
     * Chain hasil reorg harus valid.
     */
    assert(node.is_chain_valid());

    std::cout << "Reorg chain validation: OK\n";

    /*
     * UTXO fork A harus hilang.
     */
    assert(!node.utxos().exists({a1_txid, 0}));
    assert(!node.utxos().exists({a2_txid, 0}));

    std::cout << "Old fork UTXO removed: OK\n";

    /*
     * UTXO fork B harus dibangun ulang.
     */
    assert(node.utxos().exists({b1_coinbase.txid(), 0}));
    assert(node.utxos().exists({b2_coinbase.txid(), 0}));
    assert(node.utxos().exists({b3_txid, 0}));

    std::cout << "Winning fork UTXO rebuilt: OK\n";

    /*
     * Chain sekarang Genesis + B1 + B2 + B3.
     */
    assert(node.chain_size() == 4);

    std::cout << "Final chain size: OK\n";

    std::cout
        << "=== ALL FORK REORG UTXO TESTS PASSED ===\n";

    return 0;
}
