#include "src/node.h"
#include "src/genesis.h"
#include "src/transaction.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"

#include <iostream>

int main() {
    std::cout << "=== LARB NODE CHAIN SYNC TEST ===\n";

    constexpr std::uint32_t difficulty = 3;

    larb::Block genesis =
        larb::Genesis::create();

    larb::Node node_a(
        genesis,
        difficulty
    );

    larb::Node node_b(
        genesis,
        difficulty
    );

    /*
     * Block 1.
     */
    larb::Transaction coinbase1 =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-A",
            1
        );

    larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase1.serialize()},
            1700000001,
            difficulty
        );

    if (!node_a.receive_block(block1)) {
        std::cerr << "Node A block 1: FAIL\n";
        return 1;
    }

    /*
     * Block 2 hanya dimiliki Node A.
     */
    larb::Transaction coinbase2 =
        larb::Transaction::coinbase(
            larb::get_block_reward(2),
            "LARB-A",
            2
        );

    larb::Block block2 =
        larb::mine_block(
            1,
            block1.hash(),
            {coinbase2.serialize()},
            1700000002,
            difficulty
        );

    if (!node_a.receive_block(block2)) {
        std::cerr << "Node A block 2: FAIL\n";
        return 1;
    }

    /*
     * Node B hanya punya block 1.
     */
    if (!node_b.receive_block(block1)) {
        std::cerr << "Node B block 1: FAIL\n";
        return 1;
    }

    std::cout
        << "Node A chain size: "
        << node_a.chain_size()
        << '\n';

    std::cout
        << "Node B chain size: "
        << node_b.chain_size()
        << '\n';

    /*
     * Sinkronisasi.
     */
    if (!node_b.adopt_chain(
            node_a.blockchain())) {
        std::cerr
            << "Node B adopt chain: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B adopt longer chain: OK\n";

    if (node_a.chain_size() !=
        node_b.chain_size()) {
        std::cerr
            << "Chain size equality: FAIL\n";
        return 1;
    }

    std::cout
        << "Chain size equality: OK\n";

    const std::string hash_a =
        node_a.blockchain()
            .at(node_a.chain_size() - 1)
            .hash();

    const std::string hash_b =
        node_b.blockchain()
            .at(node_b.chain_size() - 1)
            .hash();

    if (hash_a != hash_b) {
        std::cerr
            << "Tip hash equality: FAIL\n";
        return 1;
    }

    std::cout
        << "Tip hash equality: OK\n";

    if (!node_b.is_chain_valid()) {
        std::cerr
            << "Node B chain validation: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B chain validation: OK\n";

    if (node_a.utxos().size() !=
        node_b.utxos().size()) {
        std::cerr
            << "UTXO equality: FAIL\n";
        return 1;
    }

    std::cout
        << "UTXO equality: OK\n";

    std::cout
        << "Node A UTXO count: "
        << node_a.utxos().size()
        << '\n';

    std::cout
        << "Node B UTXO count: "
        << node_b.utxos().size()
        << '\n';

    std::cout
        << "=== ALL NODE CHAIN SYNC TESTS PASSED ===\n";

    return 0;
}
