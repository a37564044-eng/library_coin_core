#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace larb {

class AddressCodec {
public:
    // Encode 32-byte public-key commitment menjadi:
    // larb1q...
    static std::string encode(
        const std::vector<std::uint8_t>& payload
    );

    // Decode address dan mengembalikan payload.
    // Return false jika address invalid.
    static bool decode(
        const std::string& address,
        std::vector<std::uint8_t>& payload
    );

    static bool is_valid(
        const std::string& address
    );
};

} // namespace larb
