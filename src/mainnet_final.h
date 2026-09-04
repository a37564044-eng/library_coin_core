#pragma once

#include <cstdint>
#include <string>

namespace larb {

class MainnetFinal {
public:
    static constexpr const char* NETWORK = "LARB MAINNET";

    static constexpr const char* GENESIS_HASH =
        "00032373b4b4b78d9f32723dc913bec6c6eb3a0e2a5db92a16bb0f3be4ed909a";

    static constexpr std::uint64_t GENESIS_NONCE = 1327;
    static constexpr std::uint64_t POW_DIFFICULTY = 3;

    static bool verify();
};

} // namespace larb
