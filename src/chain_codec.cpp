#include "chain_codec.h"

#include <cstddef>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace larb {

namespace {

std::string read_line(
    const std::string& data,
    std::size_t& pos
) {
    const std::size_t end = data.find('\n', pos);

    if (end == std::string::npos) {
        throw std::runtime_error("missing newline");
    }

    std::string result = data.substr(pos, end - pos);
    pos = end + 1;

    return result;
}

std::size_t read_size(
    const std::string& data,
    std::size_t& pos
) {
    const std::string value = read_line(data, pos);

    if (value.empty()) {
        throw std::runtime_error("empty size");
    }

    std::size_t used = 0;
    const unsigned long long number =
        std::stoull(value, &used);

    if (used != value.size()) {
        throw std::runtime_error("invalid size");
    }

    if (number > static_cast<unsigned long long>(
            std::numeric_limits<std::size_t>::max())) {
        throw std::runtime_error("size overflow");
    }

    return static_cast<std::size_t>(number);
}

std::string serialize_block(const Block& block) {
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

Block deserialize_block(
    const std::string& data,
    std::size_t& pos,
    std::size_t block_size
) {
    if (pos > data.size() ||
        block_size > data.size() - pos) {
        throw std::runtime_error("block exceeds payload");
    }

    const std::size_t block_end = pos + block_size;

    const std::uint32_t version =
        static_cast<std::uint32_t>(
            std::stoul(read_line(data, pos))
        );

    const std::string previous_hash =
        read_line(data, pos);

    const std::string merkle_root =
        read_line(data, pos);

    const std::uint64_t timestamp =
        std::stoull(read_line(data, pos));

    const std::uint64_t nonce =
        std::stoull(read_line(data, pos));

    const std::size_t tx_count =
        read_size(data, pos);

    std::vector<std::string> transactions;

    for (std::size_t i = 0; i < tx_count; ++i) {
        const std::size_t colon =
            data.find(':', pos);

        if (colon == std::string::npos ||
            colon >= block_end) {
            throw std::runtime_error(
                "missing transaction separator"
            );
        }

        const std::string length_text =
            data.substr(pos, colon - pos);

        if (length_text.empty()) {
            throw std::runtime_error(
                "invalid transaction length"
            );
        }

        std::size_t used = 0;

        const unsigned long long tx_size_number =
            std::stoull(length_text, &used);

        if (used != length_text.size() ||
            tx_size_number > static_cast<unsigned long long>(
                std::numeric_limits<std::size_t>::max())) {
            throw std::runtime_error(
                "invalid transaction size"
            );
        }

        const std::size_t tx_size =
            static_cast<std::size_t>(tx_size_number);

        pos = colon + 1;

        if (tx_size > block_end - pos) {
            throw std::runtime_error(
                "transaction exceeds block"
            );
        }

        std::string tx =
            data.substr(pos, tx_size);

        pos += tx_size;

        if (pos >= block_end ||
            data[pos] != '\n') {
            throw std::runtime_error(
                "missing transaction newline"
            );
        }

        ++pos;

        transactions.push_back(std::move(tx));
    }

    if (pos != block_end) {
        throw std::runtime_error(
            "block size mismatch"
        );
    }

    Block block(
        version,
        previous_hash,
        transactions,
        timestamp,
        nonce
    );

    if (block.header().merkle_root != merkle_root) {
        throw std::runtime_error(
            "merkle root mismatch"
        );
    }

    return block;
}

} // namespace

std::string serialize_chain(const Blockchain& chain) {
    std::ostringstream out;

    /*
     * Difficulty adalah bagian dari consensus state.
     * Harus ikut disimpan agar hasil decode identik.
     */
    out << chain.difficulty() << '\n';
    out << chain.size() << '\n';

    for (std::size_t i = 0; i < chain.size(); ++i) {
        const std::string block =
            serialize_block(chain.at(i));

        out << block.size() << '\n';
        out << block;
    }

    return out.str();
}

std::optional<Blockchain> deserialize_chain(
    const std::string& data
) {
    try {
        std::size_t pos = 0;

        const std::uint32_t difficulty =
            static_cast<std::uint32_t>(
                read_size(data, pos)
            );

        const std::size_t block_count =
            read_size(data, pos);

        if (block_count == 0) {
            return std::nullopt;
        }

        std::vector<Block> blocks;
        blocks.reserve(block_count);

        for (std::size_t i = 0;
             i < block_count;
             ++i) {

            const std::size_t block_size =
                read_size(data, pos);

            blocks.push_back(
                deserialize_block(
                    data,
                    pos,
                    block_size
                )
            );
        }

        if (pos != data.size()) {
            return std::nullopt;
        }

        Blockchain chain(
            blocks.front(),
            difficulty
        );

        for (std::size_t i = 1;
             i < blocks.size();
             ++i) {

            if (!chain.add_block(blocks[i])) {
                return std::nullopt;
            }
        }

        if (!chain.is_valid()) {
            return std::nullopt;
        }

        return chain;

    } catch (...) {
        return std::nullopt;
    }
}

} // namespace larb
