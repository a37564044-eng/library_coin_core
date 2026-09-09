#include "src/genesis.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"
#include "src/transaction.h"

#include <iostream>
#include <sstream>
#include <string>

std::string serialize_block(const larb::Block& block)
{
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

int main()
{
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

    const std::string wire =
        serialize_block(block);

    std::cout << "=== WIRE DEBUG ===\n";
    std::cout << "wire.size = "
              << wire.size() << '\n';

    std::cout << "tx.size = "
              << block.transactions()[0].size()
              << '\n';

    std::cout << "tx = ["
              << block.transactions()[0]
              << "]\n";

    std::cout << "\nLINES:\n";

    std::size_t start = 0;
    std::size_t line = 0;

    while (start < wire.size()) {

        std::size_t end =
            wire.find('\n', start);

        if (end == std::string::npos)
            end = wire.size();

        std::cout
            << line
            << " start="
            << start
            << " end="
            << end
            << " len="
            << (end - start)
            << " ["
            << wire.substr(
                   start,
                   end - start
               )
            << "]\n";

        if (end == wire.size())
            break;

        start = end + 1;
        ++line;
    }

    return 0;
}
