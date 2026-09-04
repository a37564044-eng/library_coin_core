#include "src/address_codec.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::cout << "=== LARB F4.1 ADDRESS CODEC TEST ===\n";

    std::vector<std::uint8_t> payload(32);

    for (std::size_t i = 0; i < payload.size(); ++i) {
        payload[i] = static_cast<std::uint8_t>(i);
    }

    const std::string address =
        larb::AddressCodec::encode(payload);

    std::cout << "Address: " << address << "\n";

    if (address.rfind("larb1q", 0) != 0) {
        std::cerr << "Canonical prefix: FAIL\n";
        return 1;
    }

    std::cout << "Canonical prefix: OK\n";

    if (!larb::AddressCodec::is_valid(address)) {
        std::cerr << "Address validation: FAIL\n";
        return 1;
    }

    std::cout << "Address validation: OK\n";

    std::vector<std::uint8_t> decoded;

    if (!larb::AddressCodec::decode(address, decoded)) {
        std::cerr << "Decode: FAIL\n";
        return 1;
    }

    if (decoded != payload) {
        std::cerr << "Round-trip payload: FAIL\n";
        return 1;
    }

    std::cout << "Round-trip payload: OK\n";

    std::string tampered = address;

    const std::size_t pos = tampered.size() - 1;

    tampered[pos] =
        tampered[pos] == 'q' ? 'p' : 'q';

    if (larb::AddressCodec::is_valid(tampered)) {
        std::cerr << "Checksum rejection: FAIL\n";
        return 1;
    }

    std::cout << "Checksum rejection: OK\n";

    std::string uppercase = address;

    for (char& c : uppercase) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
    }

    if (larb::AddressCodec::is_valid(uppercase)) {
        std::cerr << "Uppercase rejection: FAIL\n";
        return 1;
    }

    std::cout << "Uppercase rejection: OK\n";

    std::string wrong_hrp = address;

    wrong_hrp.replace(0, 4, "test");

    if (larb::AddressCodec::is_valid(wrong_hrp)) {
        std::cerr << "Wrong HRP rejection: FAIL\n";
        return 1;
    }

    std::cout << "Wrong HRP rejection: OK\n";

    std::vector<std::uint8_t> bad_payload(31);

    bool rejected = false;

    try {
        (void)larb::AddressCodec::encode(bad_payload);
    } catch (...) {
        rejected = true;
    }

    if (!rejected) {
        std::cerr << "Invalid payload size rejection: FAIL\n";
        return 1;
    }

    std::cout << "Invalid payload size rejection: OK\n";

    std::cout << "=== ALL F4.1 ADDRESS TESTS PASSED ===\n";
    return 0;
}
