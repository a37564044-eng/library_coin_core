#pragma once

#include "block.h"

#include <cstdint>

namespace larb {

bool validate_proof_of_work(
    const Block& block,
    std::uint32_t difficulty
);

Block mine_block(
    std::uint32_t version,
    const std::string& previous_hash,
    const std::vector<std::string>& transactions,
    std::uint64_t timestamp,
    std::uint32_t difficulty
);

} // namespace larb
