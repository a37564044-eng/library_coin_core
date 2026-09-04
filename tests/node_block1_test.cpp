#include "src/genesis.h"
#include "src/node.h"
#include "src/transaction.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"

#include <iostream>

int main() {
    std::cout << "=== LARB NODE BLOCK #1 TEST ===\n";

    const larb::Block genesis =
        larb::Genesis::create();

    larb::Node node(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    if (!node.is_chain_valid()) {
        std::cerr << "Initial chain: FAIL\n";
        return 1;
    }

    std::cout << "Initial chain: OK\n";

    const std::int64_t reward =
        larb::get_block_reward(1);

    if (reward != 50 * larb::COIN) {
        std::cerr << "Block reward: FAIL\n";
        return 1;
    }

    std::cout << "Block reward: "
              << reward << " satoshi: OK\n";

    const larb::Transaction coinbase =
        larb::Transaction::coinbase(
            reward,
            "LARB-MINER"
        );

    const std::string coinbase_tx =
        coinbase.serialize();

    const larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase_tx},
            2,
            larb::INITIAL_POW_DIFFICULTY
        );

    if (block1.header().previous_hash !=
        genesis.hash()) {
        std::cerr << "Previous hash: FAIL\n";
        return 1;
    }

    std::cout << "Previous hash: OK\n";

    if (block1.header().merkle_root !=
        block1.calculate_merkle_root()) {
        std::cerr << "Merkle root: FAIL\n";
        return 1;
    }

    std::cout << "Merkle root: OK\n";

    if (!larb::validate_proof_of_work(
            block1,
            larb::INITIAL_POW_DIFFICULTY)) {
        std::cerr << "PoW: FAIL\n";
        return 1;
    }

    std::cout << "PoW: OK\n";

    if (!node.receive_block(block1)) {
        std::cerr << "Node receive block #1: FAIL\n";
        return 1;
    }

    std::cout << "Node receive block #1: OK\n";

    if (node.chain_size() != 2) {
        std::cerr << "Chain size: FAIL\n";
        return 1;
    }

    std::cout << "Chain size = 2: OK\n";

    if (!node.is_chain_valid()) {
        std::cerr << "Final chain validity: FAIL\n";
        return 1;
    }

    std::cout << "Final chain validity: OK\n";

    std::cout << "Block #1 hash: "
              << block1.hash() << "\n";

    std::cout << "Block #1 nonce: "
              << block1.header().nonce << "\n";

    std::cout << "=== NODE BLOCK #1 TEST PASSED ===\n";

    return 0;
}
