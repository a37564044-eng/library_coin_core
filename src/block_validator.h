#pragma once

#include "block.h"
#include "utxo_set.h"

#include <cstdint>

namespace larb {

bool validate_block(
    const Block& block,
    std::uint64_t height,
    UTXOSet& utxos
);

} // namespace larb
