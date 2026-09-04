#pragma once

#include "transaction.h"

#include <cstdint>
#include <string>

namespace larb {

struct OutPoint {
    std::string txid;
    std::uint32_t output_index;

    bool operator==(const OutPoint& other) const {
        return txid == other.txid &&
               output_index == other.output_index;
    }
};

struct UTXO {
    OutPoint outpoint;
    TransactionOutput output;
};

} // namespace larb
