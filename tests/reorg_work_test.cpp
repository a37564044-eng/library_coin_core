#include "blockchain.h"
#include "genesis.h"
#include "consensus/pow.h"
#include "consensus/constants.h"
#include "node.h"
#include "transaction.h"

#include <iostream>

int main() {
    std::cout << "=== LARB REORG WORK TEST ===\n";

    constexpr std::uint32_t difficulty = 3;

    const larb::Block genesis =
        larb::Genesis::create();

    larb::Node node_a(genesis, difficulty);
    larb::Node node_b(genesis, difficulty);

    larb::Transaction tx1 =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-A",
            1
        );

    larb::Block a1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {tx1.serialize()},
            1700000001,
            difficulty
        );

    if (!node_a.receive_block(a1)) {
        std::cerr << "A block 1: FAIL\n";
        return 1;
    }

    larb::Transaction tx2 =
        larb::Transaction::coinbase(
            larb::get_block_reward(2),
            "LARB-A",
            2
        );

    larb::Block a2 =
        larb::mine_block(
            1,
            a1.hash(),
            {tx2.serialize()},
            1700000002,
            difficulty
        );

    if (!node_a.receive_block(a2)) {
        std::cerr << "A block 2: FAIL\n";
        return 1;
    }

    if (!node_b.receive_block(a1)) {
        std::cerr << "B block 1: FAIL\n";
        return 1;
    }

    std::cout << "Node A work: "
              << static_cast<unsigned long long>(
                     node_a.blockchain().chain_work())
              << '\n';

    std::cout << "Node B work: "
              << static_cast<unsigned long long>(
                     node_b.blockchain().chain_work())
              << '\n';

    if (!node_b.adopt_chain(node_a.blockchain())) {
        std::cerr << "Adopt higher-work chain: FAIL\n";
        return 1;
    }

    std::cout << "Adopt higher-work chain: OK\n";

    if (node_b.chain_size() != node_a.chain_size()) {
        std::cerr << "Chain size equality: FAIL\n";
        return 1;
    }

    if (node_b.blockchain()
            .at(node_b.chain_size() - 1)
            .hash()
        !=
        node_a.blockchain()
            .at(node_a.chain_size() - 1)
            .hash()) {
        std::cerr << "Tip equality: FAIL\n";
        return 1;
    }

    std::cout << "Tip equality: OK\n";

    if (!node_b.is_chain_valid()) {
        std::cerr << "Reorg chain validation: FAIL\n";
        return 1;
    }

    std::cout << "Reorg chain validation: OK\n";

    std::cout
        << "=== ALL REORG WORK TESTS PASSED ===\n";

    return 0;
}
