#include "src/genesis.h"
#include "src/node.h"
#include "src/transaction.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"

#include <iostream>

int main() {
    std::cout << "=== LARB NODE TEST ===\n";

    constexpr std::uint32_t difficulty =
        larb::INITIAL_POW_DIFFICULTY;

    larb::Block genesis =
        larb::Genesis::create();

    larb::Node node(genesis, difficulty);

    if (node.chain_size() != 1) {
        std::cerr << "Genesis chain: FAIL\n";
        return 1;
    }

    std::cout << "Genesis chain: OK\n";

    /*
     * Block 1 wajib mempunyai coinbase.
     */
    const larb::Transaction coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-MINER",
            1
        );

    const std::string coinbase_tx =
        coinbase.serialize();

    larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase_tx},
            2,
            difficulty
        );

    if (!larb::validate_proof_of_work(
            block1,
            difficulty)) {
        std::cerr << "Block 1 PoW: FAIL\n";
        return 1;
    }

    if (!node.receive_block(block1)) {
        std::cerr << "Receive valid mined block: FAIL\n";
        return 1;
    }

    std::cout << "Receive valid mined block: OK\n";

    /*
     * Block dengan previous hash salah.
     */
    larb::Block invalid_previous =
        larb::mine_block(
            1,
            "WRONG_PREVIOUS_HASH",
            {coinbase_tx},
            3,
            difficulty
        );

    if (node.receive_block(invalid_previous)) {
        std::cerr << "Reject invalid previous hash: FAIL\n";
        return 1;
    }

    std::cout << "Reject invalid previous hash: OK\n";

    /*
     * Block dengan PoW salah.
     */
    larb::Block invalid_pow(
        1,
        block1.hash(),
        {coinbase_tx},
        4,
        block1.header().nonce + 1
    );

    if (node.receive_block(invalid_pow)) {
        std::cerr << "Reject invalid PoW: FAIL\n";
        return 1;
    }

    std::cout << "Reject invalid PoW: OK\n";

    if (!node.is_chain_valid()) {
        std::cerr << "Node chain validation: FAIL\n";
        return 1;
    }

    std::cout << "Node chain validation: OK\n";

    if (node.chain_size() != 2) {
        std::cerr << "Final chain size: FAIL\n";
        return 1;
    }

    std::cout << "Final chain size: OK\n";

    std::cout << "UTXO count: "
              << node.utxos().size()
              << '\n';

    std::cout << "=== ALL NODE TESTS PASSED ===\n";

    return 0;
}
