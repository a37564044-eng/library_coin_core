#include "pqc.h"

extern "C" {
#include <oqs/sig_ml_dsa.h>
}

#include <stdexcept>
#include <vector>
#include <cstddef>

namespace larb {

namespace {

std::string bytes_to_hex(
    const unsigned char* data,
    std::size_t size
) {
    static const char* hex = "0123456789abcdef";

    std::string out;
    out.resize(size * 2);

    for (std::size_t i = 0; i < size; ++i) {
        out[i * 2] =
            hex[(data[i] >> 4) & 0x0f];

        out[i * 2 + 1] =
            hex[data[i] & 0x0f];
    }

    return out;
}

unsigned char hex_value(char c) {
    if (c >= '0' && c <= '9')
        return static_cast<unsigned char>(c - '0');

    if (c >= 'a' && c <= 'f')
        return static_cast<unsigned char>(c - 'a' + 10);

    if (c >= 'A' && c <= 'F')
        return static_cast<unsigned char>(c - 'A' + 10);

    throw std::invalid_argument(
        "invalid hexadecimal character"
    );
}

std::string hex_to_bytes(
    const std::string& hex
) {
    if (hex.size() % 2 != 0) {
        throw std::invalid_argument(
            "hex string must have even length"
        );
    }

    std::string out;
    out.resize(hex.size() / 2);

    for (std::size_t i = 0; i < out.size(); ++i) {
        const unsigned char high =
            hex_value(hex[i * 2]);

        const unsigned char low =
            hex_value(hex[i * 2 + 1]);

        out[i] = static_cast<char>(
            (high << 4) | low
        );
    }

    return out;
}

} // namespace

PQCKeyPair PQC::generate_keypair() {
    std::vector<unsigned char> public_key(
        OQS_SIG_ml_dsa_44_length_public_key
    );

    std::vector<unsigned char> private_key(
        OQS_SIG_ml_dsa_44_length_secret_key
    );

    if (OQS_SIG_ml_dsa_44_keypair(
            public_key.data(),
            private_key.data()
        ) != OQS_SUCCESS) {

        throw std::runtime_error(
            "ML-DSA-44 keypair generation failed"
        );
    }

    return {
        std::string(
            reinterpret_cast<const char*>(
                public_key.data()
            ),
            public_key.size()
        ),

        std::string(
            reinterpret_cast<const char*>(
                private_key.data()
            ),
            private_key.size()
        )
    };
}

std::string PQC::sign(
    const std::string& message,
    const std::string& private_key
) {
    if (
        private_key.size() !=
        OQS_SIG_ml_dsa_44_length_secret_key
    ) {
        throw std::runtime_error(
            "invalid ML-DSA-44 private key size"
        );
    }

    std::vector<unsigned char> signature(
        OQS_SIG_ml_dsa_44_length_signature
    );

    size_t signature_len = 0;

    if (OQS_SIG_ml_dsa_44_sign(
            signature.data(),
            &signature_len,
            reinterpret_cast<const unsigned char*>(
                message.data()
            ),
            message.size(),
            reinterpret_cast<const unsigned char*>(
                private_key.data()
            )
        ) != OQS_SUCCESS) {

        throw std::runtime_error(
            "ML-DSA-44 signing failed"
        );
    }

    return std::string(
        reinterpret_cast<const char*>(
            signature.data()
        ),
        signature_len
    );
}

bool PQC::verify(
    const std::string& message,
    const std::string& signature,
    const std::string& public_key
) {
    if (
        public_key.size() !=
        OQS_SIG_ml_dsa_44_length_public_key
    ) {
        return false;
    }

    if (
        signature.size() !=
        OQS_SIG_ml_dsa_44_length_signature
    ) {
        return false;
    }

    return OQS_SIG_ml_dsa_44_verify(
        reinterpret_cast<const unsigned char*>(
            message.data()
        ),
        message.size(),

        reinterpret_cast<const unsigned char*>(
            signature.data()
        ),
        signature.size(),

        reinterpret_cast<const unsigned char*>(
            public_key.data()
        )
    ) == OQS_SUCCESS;
}

std::string PQC::sign_hex(
    const std::string& message,
    const std::string& private_key_hex
) {
    const std::string private_key =
        hex_to_bytes(private_key_hex);

    const std::string signature =
        sign(message, private_key);

    return bytes_to_hex(
        reinterpret_cast<const unsigned char*>(
            signature.data()
        ),
        signature.size()
    );
}

bool PQC::verify_hex(
    const std::string& message,
    const std::string& signature_hex,
    const std::string& public_key_hex
) {
    try {
        const std::string signature =
            hex_to_bytes(signature_hex);

        const std::string public_key =
            hex_to_bytes(public_key_hex);

        return verify(
            message,
            signature,
            public_key
        );
    }
    catch (...) {
        return false;
    }
}

} // namespace larb
