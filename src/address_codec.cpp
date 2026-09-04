#include "address_codec.h"

#include <array>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace larb {

namespace {

constexpr char CHARSET[] =
    "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

constexpr std::uint32_t GENERATOR[] = {
    0x3b6a57b2,
    0x26508e6d,
    0x1ea119fa,
    0x3d4233dd,
    0x2a1462b3
};

std::uint32_t polymod(
    const std::vector<std::uint8_t>& values
) {
    std::uint32_t chk = 1;

    for (const std::uint8_t value : values) {
        const std::uint8_t top =
            static_cast<std::uint8_t>(chk >> 25);

        chk = ((chk & 0x1ffffffU) << 5) ^ value;

        for (int i = 0; i < 5; ++i) {
            if ((top >> i) & 1U) {
                chk ^= GENERATOR[i];
            }
        }
    }

    return chk;
}

std::vector<std::uint8_t> hrp_expand(
    const std::string& hrp
) {
    std::vector<std::uint8_t> result;

    result.reserve(hrp.size() * 2 + 1);

    for (const unsigned char c : hrp) {
        result.push_back(c >> 5);
    }

    result.push_back(0);

    for (const unsigned char c : hrp) {
        result.push_back(c & 31);
    }

    return result;
}

std::vector<std::uint8_t> checksum(
    const std::string& hrp,
    const std::vector<std::uint8_t>& data
) {
    std::vector<std::uint8_t> values =
        hrp_expand(hrp);

    values.insert(
        values.end(),
        data.begin(),
        data.end()
    );

    values.insert(
        values.end(),
        6,
        0
    );

    const std::uint32_t mod =
        polymod(values) ^ 1U;

    std::vector<std::uint8_t> result(6);

    for (int i = 0; i < 6; ++i) {
        result[i] =
            static_cast<std::uint8_t>(
                (mod >> (5 * (5 - i))) & 31U
            );
    }

    return result;
}

int charset_rev(char c) {
    static const std::array<int, 128> table = [] {
        std::array<int, 128> t{};
        t.fill(-1);

        for (int i = 0; CHARSET[i] != '\0'; ++i) {
            t[static_cast<unsigned char>(CHARSET[i])] = i;
        }

        return t;
    }();

    const unsigned char uc =
        static_cast<unsigned char>(c);

    if (uc >= 128) {
        return -1;
    }

    return table[uc];
}

bool convert_bits(
    const std::vector<std::uint8_t>& input,
    int from_bits,
    int to_bits,
    bool pad,
    std::vector<std::uint8_t>& output
) {
    std::uint32_t acc = 0;
    int bits = 0;

    const std::uint32_t maxv =
        (1U << to_bits) - 1U;

    const std::uint32_t max_acc =
        (1U << (from_bits + to_bits - 1)) - 1U;

    output.clear();

    for (const std::uint8_t value : input) {
        if ((value >> from_bits) != 0) {
            return false;
        }

        acc =
            ((acc << from_bits) | value) &
            max_acc;

        bits += from_bits;

        while (bits >= to_bits) {
            bits -= to_bits;

            output.push_back(
                static_cast<std::uint8_t>(
                    (acc >> bits) & maxv
                )
            );
        }
    }

    if (pad) {
        if (bits > 0) {
            output.push_back(
                static_cast<std::uint8_t>(
                    (acc << (to_bits - bits)) &
                    maxv
                )
            );
        }
    } else {
        if (bits >= from_bits) {
            return false;
        }

        if (((acc << (to_bits - bits)) &
             maxv) != 0) {
            return false;
        }
    }

    return true;
}

std::string encode_bech32(
    const std::string& hrp,
    const std::vector<std::uint8_t>& data
) {
    std::vector<std::uint8_t> values = data;

    const std::vector<std::uint8_t> sum =
        checksum(hrp, data);

    values.insert(
        values.end(),
        sum.begin(),
        sum.end()
    );

    std::string result = hrp;
    result.push_back('1');

    for (const std::uint8_t value : values) {
        if (value >= 32) {
            throw std::runtime_error(
                "invalid bech32 value"
            );
        }

        result.push_back(CHARSET[value]);
    }

    return result;
}

bool decode_bech32(
    const std::string& address,
    std::string& hrp,
    std::vector<std::uint8_t>& data
) {
    if (address.size() < 8 ||
        address.size() > 90) {
        return false;
    }

    bool has_lower = false;
    bool has_upper = false;

    for (const unsigned char c : address) {
        if (c < 33 || c > 126) {
            return false;
        }

        if (std::islower(c)) {
            has_lower = true;
        }

        if (std::isupper(c)) {
            has_upper = true;
        }
    }

    // Mixed case is forbidden.
    if (has_lower && has_upper) {
        return false;
    }

    // LARB protocol uses lowercase addresses.
    if (has_upper) {
        return false;
    }

    const std::size_t separator =
        address.rfind('1');

    if (separator == std::string::npos ||
        separator < 1 ||
        separator + 7 > address.size()) {
        return false;
    }

    hrp = address.substr(0, separator);

    if (hrp != "larb") {
        return false;
    }

    data.clear();

    for (std::size_t i = separator + 1;
         i < address.size();
         ++i) {

        const int value =
            charset_rev(address[i]);

        if (value < 0) {
            return false;
        }

        data.push_back(
            static_cast<std::uint8_t>(value)
        );
    }

    if (data.size() < 7) {
        return false;
    }

    const std::vector<std::uint8_t>
        expanded = hrp_expand(hrp);

    std::vector<std::uint8_t> values =
        expanded;

    values.insert(
        values.end(),
        data.begin(),
        data.end()
    );

    if (polymod(values) != 1U) {
        return false;
    }

    data.resize(data.size() - 6);

    return true;
}

} // namespace

std::string AddressCodec::encode(
    const std::vector<std::uint8_t>& payload
) {
    if (payload.size() != 32) {
        throw std::invalid_argument(
            "LARB address payload must be 32 bytes"
        );
    }

    std::vector<std::uint8_t> converted;

    if (!convert_bits(
            payload,
            8,
            5,
            true,
            converted)) {
        throw std::runtime_error(
            "address convert_bits failed"
        );
    }

    // Version 0 => Bech32 data starts with q.
    std::vector<std::uint8_t> data;
    data.reserve(1 + converted.size());

    data.push_back(0);
    data.insert(
        data.end(),
        converted.begin(),
        converted.end()
    );

    return encode_bech32(
        "larb",
        data
    );
}

bool AddressCodec::decode(
    const std::string& address,
    std::vector<std::uint8_t>& payload
) {
    std::string hrp;
    std::vector<std::uint8_t> data;

    if (!decode_bech32(
            address,
            hrp,
            data)) {
        return false;
    }

    // Version 0.
    if (data.empty() || data[0] != 0) {
        return false;
    }

    std::vector<std::uint8_t> converted(
        data.begin() + 1,
        data.end()
    );

    if (!convert_bits(
            converted,
            5,
            8,
            false,
            payload)) {
        return false;
    }

    if (payload.size() != 32) {
        return false;
    }

    return true;
}

bool AddressCodec::is_valid(
    const std::string& address
) {
    std::vector<std::uint8_t> payload;

    return decode(
        address,
        payload
    );
}

} // namespace larb
