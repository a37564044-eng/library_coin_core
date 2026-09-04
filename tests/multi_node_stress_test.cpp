#include "genesis.h"
#include "node.h"
#include "transaction.h"
#include "consensus/pow.h"
#include "consensus/constants.h"

#include <cassert>
#include <iostream>

static larb::Block mine(
    const larb::Block& prev,
    std::uint64_t height,
    const std::string& address,
    std::uint64_t timestamp
) {
    auto tx = larb::Transaction::coinbase(
        larb::get_block_reward(height),
        address,
        height
    );

    return larb::mine_block(
        1,
        prev.hash(),
        {tx.serialize()},
        timestamp,
        3
    );
}

int main() {
    constexpr std::uint32_t difficulty = 3;

    const std::string address =
        "larb1q02gwnsand4u9k9negtwe8zdfwknzedu33e76reqzac6vh6efgx9slg8793";

    const larb::Block genesis = larb::Genesis::create();

    larb::Node node_a(genesis, difficulty);
    larb::Node node_b(genesis, difficulty);
    larb::Node node_c(genesis, difficulty);

    larb::Block prev = genesis;

    // Node A membuat chain 3 blok.
    for (std::uint64_t h = 1; h <= 3; ++h) {
        auto block = mine(
            prev, h, address,
            1700000000ULL + h
        );

        assert(node_a.receive_block(block));
        prev = block;
    }

    assert(node_a.chain_size() == 4);
    assert(node_a.is_chain_valid());

    // Node B dan C mengadopsi chain A.
    assert(node_b.adopt_chain(node_a.blockchain()));
    assert(node_c.adopt_chain(node_a.blockchain()));

    assert(node_b.chain_size() == 4);
    assert(node_c.chain_size() == 4);

    // Node B membuat chain yang lebih panjang.
    larb::Blockchain longer = node_b.blockchain();

    auto block4 = mine(
        longer.at(3),
        4,
        address,
        1700000010ULL
    );
    assert(longer.add_block(block4));

    auto block5 = mine(
        block4,
        5,
        address,
        1700000011ULL
    );
    assert(longer.add_block(block5));

    // Node B menyimpan chain terpanjang.
    // Node C menerima chain yang lebih berat.
    assert(node_b.adopt_chain(longer));
    assert(node_b.chain_size() == 6);
    assert(node_b.is_chain_valid());
    assert(node_c.adopt_chain(longer));
    assert(node_c.chain_size() == 6);
    assert(node_c.is_chain_valid());

    // Node A masih chain lama dan ikut sinkron.
    assert(node_a.adopt_chain(longer));
    assert(node_a.chain_size() == 6);
    assert(node_a.is_chain_valid());

    // Semua node sekarang identik.
    assert(node_a.blockchain().at(5).hash() ==
           node_b.blockchain().at(5).hash());

    assert(node_b.blockchain().at(5).hash() ==
           node_c.blockchain().at(5).hash());

    std::cout << "Multi-node stress test: PASS\n";
    return 0;
}
