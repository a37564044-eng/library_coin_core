#include "node.h"
#include "genesis.h"
#include "transaction.h"
#include "consensus/pow.h"
#include "consensus/constants.h"

#include <cassert>
#include <iostream>

int main() {
    constexpr std::uint32_t difficulty = 3;
    const larb::Block genesis = larb::Genesis::create();

    const std::string address =
        "larb1q02gwnsand4u9k9negtwe8zdfwknzedu33e76reqzac6vh6efgx9slg8793";

    for (int round = 1; round <= 10; ++round) {
        larb::Node node(genesis, difficulty);

        larb::Blockchain fork_a(genesis, difficulty);
        larb::Blockchain fork_b(genesis, difficulty);

        larb::Block prev_a = genesis;
        larb::Block prev_b = genesis;

        const std::uint64_t base_time =
            1700000000ULL + static_cast<std::uint64_t>(round) * 100;

        for (std::uint64_t h = 1; h <= 3; ++h) {
            auto tx_a = larb::Transaction::coinbase(
                larb::get_block_reward(h), address, h);

            auto a = larb::mine_block(
                1, prev_a.hash(), {tx_a.serialize()},
                base_time + h, difficulty);

            assert(fork_a.add_block(a));
            prev_a = a;

            auto tx_b = larb::Transaction::coinbase(
                larb::get_block_reward(h), address, h);

            auto b = larb::mine_block(
                1, prev_b.hash(), {tx_b.serialize()},
                base_time + h + 10, difficulty);

            assert(fork_b.add_block(b));
            prev_b = b;
        }

        // A masuk lebih dulu.
        assert(node.adopt_chain(fork_a));
        assert(node.chain_size() == 4);

        // B sama panjang -> wajib ditolak.
        assert(!node.adopt_chain(fork_b));
        assert(node.chain_size() == 4);
        assert(node.is_chain_valid());

        // B menjadi lebih panjang.
        auto tx_b4 = larb::Transaction::coinbase(
            larb::get_block_reward(4), address, 4);

        auto b4 = larb::mine_block(
            1, prev_b.hash(), {tx_b4.serialize()},
            base_time + 20, difficulty);

        assert(fork_b.add_block(b4));

        assert(fork_b.chain_work() >
               node.blockchain().chain_work());

        // Reorg A -> B.
        assert(node.adopt_chain(fork_b));
        assert(node.chain_size() == 5);
        assert(node.is_chain_valid());
    }

    std::cout << "Reorg stress test: PASS\n";
    return 0;
}
