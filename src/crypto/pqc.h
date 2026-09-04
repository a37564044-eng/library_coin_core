#pragma once

#include <string>

namespace larb {

struct PQCKeyPair {
    std::string public_key;
    std::string private_key;
};

class PQC {
public:
    static PQCKeyPair generate_keypair();

    static std::string sign(
        const std::string& message,
        const std::string& private_key
    );

    static bool verify(
        const std::string& message,
        const std::string& signature,
        const std::string& public_key
    );

    static std::string sign_hex(
        const std::string& message,
        const std::string& private_key_hex
    );

    static bool verify_hex(
        const std::string& message,
        const std::string& signature_hex,
        const std::string& public_key_hex
    );
};

} // namespace larb
