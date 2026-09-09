#include "src/genesis.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"
#include "src/block.h"
#include "src/transaction.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

larb::Block deserialize_single_block_debug(
    const std::string& data
) {
    std::size_t pos = 0;

    auto read_line = [&]() -> std::string {
        const std::size_t end =
            data.find('\n', pos);

        if (end == std::string::npos) {
            throw std::runtime_error(
                "missing newline"
            );
        }

        std::string result =
            data.substr(pos, end - pos);

        pos = end + 1;
        return result;
    };

    const std::uint32_t version =
        static_cast<std::uint32_t>(
            std::stoul(read_line())
        );

    const std::string previous_hash =
        read_line();

    const std::string merkle_root =
        read_line();

    const std::uint64_t timestamp =
        std::stoull(read_line());

    const std::uint64_t nonce =
        std::stoull(read_line());

    const std::size_t tx_count =
        static_cast<std::size_t>(
            std::stoull(read_line())
        );

    std::vector<std::string> transactions;

    for (std::size_t i = 0;
         i < tx_count;
         ++i) {

        const std::string length_text =
            read_line();

        const std::size_t colon =
            length_text.find(':');

        if (colon == std::string::npos) {
            throw std::runtime_error(
                "invalid transaction"
            );
        }

        const std::size_t tx_size =
            std::stoull(
                length_text.substr(
                    0,
                    colon
                )
            );

        const std::size_t tx_start = pos;

        if (tx_size >
            data.size() - tx_start) {
            throw std::runtime_error(
                "transaction truncated"
            );
        }

        std::string tx =
            data.substr(
                tx_start,
                tx_size
            );

        pos += tx_size;

        if (pos >= data.size() ||
            data[pos] != '\n') {
            throw std::runtime_error(
                "missing transaction delimiter"
            );
        }

        ++pos;

        transactions.push_back(
            std::move(tx)
        );
    }

    if (pos != data.size()) {
        throw std::runtime_error(
            "unexpected trailing data"
        );
    }

    larb::Block block(
        version,
        previous_hash,
        transactions,
        timestamp,
        nonce
    );

    if (block.header().merkle_root !=
        merkle_root) {
        throw std::runtime_error(
            "merkle root mismatch"
        );
    }

    return block;
}

std::string serialize_block(
    const larb::Block& block
) {
    const auto& h = block.header();

    std::ostringstream out;

    out << h.version << '\n';
    out << h.previous_hash << '\n';
    out << h.merkle_root << '\n';
    out << h.timestamp << '\n';
    out << h.nonce << '\n';

    const auto& txs =
        block.transactions();

    out << txs.size() << '\n';

    for (const auto& tx : txs) {
        out << tx.size()
            << ':'
            << tx
            << '\n';
    }

    return out.str();
}

}

int main() {
    std::cout
        << "=== P2P PARSER DEBUG ===\n";

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

    const std::string wire =
        serialize_block(original);

    std::cout
        << "Wire size: "
        << wire.size()
        << '\n';

    try {
        larb::Block decoded =
            deserialize_single_block_debug(
                wire
            );

        std::cout
            << "Deserialize: OK\n";

        std::cout
            << "Original hash: "
            << original.hash()
            << '\n';

        std::cout
            << "Decoded hash:  "
            << decoded.hash()
            << '\n';

        std::cout
            << "Hash equal: "
            << (original.hash() ==
                    decoded.hash()
                    ? "YES"
                    : "NO")
            << '\n';

        std::cout
            << "Previous equal: "
            << (original.header().previous_hash ==
                    decoded.header().previous_hash
                    ? "YES"
                    : "NO")
            << '\n';

        std::cout
            << "Merkle equal: "
            << (original.header().merkle_root ==
                    decoded.header().merkle_root
                    ? "YES"
                    : "NO")
            << '\n';

        std::cout
            << "Timestamp equal: "
            << (original.header().timestamp ==
                    decoded.header().timestamp
                    ? "YES"
                    : "NO")
            << '\n';

        std::cout
            << "Nonce equal: "
            << (original.header().nonce ==
                    decoded.header().nonce
                    ? "YES"
                    : "NO")
            << '\n';

        std::cout
            << "TX count equal: "
            << (original.transactions().size() ==
                    decoded.transactions().size()
                    ? "YES"
                    : "NO")
            << '\n';

        std::cout
            << "Decoded tx size: "
            << decoded.transactions()[0].size()
            << '\n';

        std::cout
            << "Decoded txid: "
            << larb::Transaction::deserialize(
                   decoded.transactions()[0]
               ).txid()
            << '\n';

        std::cout
            << "Original txid: "
            << coinbase.txid()
            << '\n';

    } catch (const std::exception& e) {
        std::cout
            << "Deserialize: FAIL\n";

        std::cout
            << "Exception: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}
