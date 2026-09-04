#include "pow.h"

#include <limits>
#include <stdexcept>

namespace larb {

namespace {

bool has_leading_zeroes(
    const std::string& hash,
    std::uint32_t difficulty
) {
    if (difficulty > hash.size()) {
        return false;
    }

    for (std::uint32_t i = 0; i < difficulty; ++i) {
        if (hash[i] != '0') {
            return false;
        }
    }

    return true;
}

} // namespace

bool validate_proof_of_work(
    const Block& block,
    std::uint32_t difficulty
) {
    return has_leading_zeroes(
        block.hash(),
        difficulty
    );
}

Block mine_block(
    std::uint32_t version,
    const std::string& previous_hash,
    const std::vector<std::string>& transactions,
    std::uint64_t timestamp,
    std::uint32_t difficulty
) {
    if (difficulty > 64) {
        throw std::invalid_argument(
            "difficulty too large"
        );
    }

    for (
        std::uint64_t nonce = 0;
        nonce < std::numeric_limits<std::uint64_t>::max();
        ++nonce
    ) {
        Block block(
            version,
            previous_hash,
            transactions,
            timestamp,
            nonce
        );

        if (validate_proof_of_work(
                block,
                difficulty)) {
            return block;
        }
    }

    throw std::runtime_error(
        "nonce space exhausted"
    );
}

} // namespace larb
