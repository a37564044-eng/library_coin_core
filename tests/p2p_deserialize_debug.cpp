#include "src/genesis.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"
#include "src/p2p.h"
#include "src/node.h"
#include "src/transaction.h"

#include <iostream>
#include <sstream>

namespace {

std::string serialize_block(const larb::Block& block) {
    const auto& h = block.header();

    std::ostringstream out;

    out << h.version << '\n';
    out << h.previous_hash << '\n';
    out << h.merkle_root << '\n';
    out << h.timestamp << '\n';
    out << h.nonce << '\n';

    const auto& txs = block.transactions();

    out << txs.size() << '\n';

    for (const auto& tx : txs) {
        out << tx.size() << ':' << tx << '\n';
    }

    return out.str();
}

}

int main() {
    std::cout << "=== P2P DESERIALIZE DEBUG ===\n";

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

    larb::Block original =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase.serialize()},
            2,
            3
        );

    larb::Node node(genesis, 3);

    larb::P2PServer server(18448);
    server.set_node(&node);

    std::string response;

    const std::string message =
        "SUBMIT_BLOCK\n" + serialize_block(original);

    const bool accepted =
        server.handle_message(message, response);

    std::cout
        << "Original hash: "
        << original.hash()
        << '\n';

    std::cout
        << "Original TX count: "
        << original.transactions().size()
        << '\n';

    std::cout
        << "Original TX size: "
        << original.transactions()[0].size()
        << '\n';

    std::cout
        << "P2P accepted: "
        << (accepted ? "YES" : "NO")
        << '\n';

    std::cout
        << "Response: ["
        << response
        << "]\n";

    std::cout
        << "Chain size: "
        << node.chain_size()
        << '\n';

    return 0;
}
