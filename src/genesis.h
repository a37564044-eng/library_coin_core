#pragma once

#include "block.h"

namespace larb {

class Genesis {
public:
    static constexpr const char* MESSAGE =
        "28/8/2026 The Times Indonesia - Ricuh Demo Senayan";

    static constexpr std::uint64_t TIMESTAMP = 1;
    static constexpr std::uint64_t NONCE = 1327;

    static Block create();
};

} // namespace larb
