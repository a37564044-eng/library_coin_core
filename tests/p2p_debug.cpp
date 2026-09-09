#include "src/genesis.h"
#include "src/node.h"
#include "src/transaction.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"

#include <iostream>

int main() {
    larb::Block genesis =
        larb::Genesis::create(
            "LARB GENESIS",
            1,
            0
        );

    constexpr std::uint32_t difficulty =
        larb::INITIAL_POW_DIFFICULTY;

    larb::Node node(genesis, difficulty);

    const larb::Transaction coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-MINER"
        );

    const std::string coinbase_tx =
        coinbase.serialize();

    larb::Block block =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase_tx},
            2,
            difficulty
        );

    std::cout << "=== P2P DEBUG ===\n";

    std::cout << "Previous hash: "
              << (block.header().previous_hash ==
                  genesis.hash()
                      ? "OK" : "FAIL")
              << "\n";

    std::cout << "PoW: "
              << (larb::validate_proof_of_work(
                      block,
                      difficulty)
                      ? "OK" : "FAIL")
              << "\n";

    std::cout << "Merkle: "
              << (block.header().merkle_root ==
                  block.calculate_merkle_root()
                      ? "OK" : "FAIL")
              << "\n";

    std::cout << "TX count: "
              << block.transactions().size()
              << "\n";

    std::cout << "TX size: "
              << block.transactions()[0].size()
              << "\n";

    std::cout << "TX newline bytes: ";

    std::size_t newline_count = 0;

    for (unsigned char c :
         block.transactions()[0]) {
        if (c == '\n') {
            ++newline_count;
        }
    }

    std::cout << newline_count << "\n";

    std::cout << "Node receive: "
              << (node.receive_block(block)
                      ? "OK"
                      : "FAIL")
              << "\n";

    std::cout << "Chain size: "
              << node.chain_size()
              << "\n";
}
