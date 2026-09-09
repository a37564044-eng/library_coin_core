#include "src/genesis.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"
#include "src/block_validator.h"
#include "src/transaction.h"
#include "src/node.h"

#include <iostream>

int main() {
    std::cout << "=== NODE RECEIVE DEBUG ===\n";

    larb::Block genesis =
        larb::Genesis::create(
            "LARB GENESIS",
            1,
            0
        );

    const larb::Transaction coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-MINER"
        );

    larb::Block block =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase.serialize()},
            2,
            3
        );

    larb::Node node(genesis, 3);

    std::cout
        << "Previous hash: "
        << (block.header().previous_hash ==
            genesis.hash() ? "OK" : "FAIL")
        << '\n';

    std::cout
        << "PoW: "
        << (larb::validate_proof_of_work(
                block, 3) ? "OK" : "FAIL")
        << '\n';

    larb::UTXOSet test_utxos;

    std::cout
        << "validate_block: "
        << (larb::validate_block(
                block,
                1,
                test_utxos) ? "OK" : "FAIL")
        << '\n';

    std::cout
        << "Node receive: "
        << (node.receive_block(block) ? "OK" : "FAIL")
        << '\n';

    std::cout
        << "Chain size: "
        << node.chain_size()
        << '\n';

    return 0;
}
