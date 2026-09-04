#include "script.h"

#include <stdexcept>
#include <openssl/sha.h>

namespace larb {

static constexpr unsigned char OP_RETURN = 0x6a;
static constexpr unsigned char OP_PUSHDATA1 = 0x4c;

std::string Script::op_return(const std::string& data) {
    if (data.size() > OP_RETURN_MAX_DATA) {
        throw std::runtime_error(
            "OP_RETURN data exceeds 80 bytes"
        );
    }

    std::string script;

    // OP_RETURN
    script.push_back(static_cast<char>(OP_RETURN));

    // Push data
    if (data.size() <= 75) {
        script.push_back(
            static_cast<char>(data.size())
        );
    } else {
        // OP_PUSHDATA1
        script.push_back(static_cast<char>(OP_PUSHDATA1));
        script.push_back(
            static_cast<char>(data.size())
        );
    }

    script.append(data);

    return script;
}

std::string Script::pay_to_pubkey_hash(const std::string& public_key) {
    if (public_key.empty()) {
        throw std::runtime_error("empty public key");
    }

    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(
        reinterpret_cast<const unsigned char*>(public_key.data()),
        public_key.size(),
        digest
    );

    std::string script;
    script.push_back(static_cast<char>(0x76));
    script.push_back(static_cast<char>(0xa9));
    script.push_back(static_cast<char>(SHA256_DIGEST_LENGTH));
    script.append(
        reinterpret_cast<const char*>(digest),
        SHA256_DIGEST_LENGTH
    );
    script.push_back(static_cast<char>(0x88));
    script.push_back(static_cast<char>(0xac));

    return script;
}

bool Script::is_pay_to_pubkey_hash(const std::string& script) {
    return script.size() == 37 &&
           static_cast<unsigned char>(script[0]) == 0x76 &&
           static_cast<unsigned char>(script[1]) == 0xa9 &&
           static_cast<unsigned char>(script[2]) == 32 &&
           static_cast<unsigned char>(script[35]) == 0x88 &&
           static_cast<unsigned char>(script[36]) == 0xac;
}

std::string Script::extract_pubkey_hash(const std::string& script) {
    if (!is_pay_to_pubkey_hash(script)) {
        throw std::runtime_error("not a valid pay-to-pubkey-hash script");
    }

    return script.substr(3, SHA256_DIGEST_LENGTH);
}


bool Script::is_op_return(
    const std::string& script
) {
    return !script.empty() &&
           static_cast<unsigned char>(script[0]) == OP_RETURN;
}

std::string Script::extract_op_return_data(
    const std::string& script
) {
    if (!is_op_return(script)) {
        throw std::runtime_error(
            "script is not OP_RETURN"
        );
    }

    if (script.size() < 2) {
        throw std::runtime_error(
            "invalid OP_RETURN script"
        );
    }

    const unsigned char opcode =
        static_cast<unsigned char>(script[1]);

    std::size_t data_offset = 2;
    std::size_t data_size = 0;

    if (opcode <= 75) {
        data_size = opcode;
    } else if (opcode == OP_PUSHDATA1) {
        if (script.size() < 3) {
            throw std::runtime_error(
                "invalid OP_PUSHDATA1 script"
            );
        }

        data_size =
            static_cast<unsigned char>(script[2]);

        data_offset = 3;
    } else {
        throw std::runtime_error(
            "unsupported OP_RETURN push opcode"
        );
    }

    if (data_size > OP_RETURN_MAX_DATA) {
        throw std::runtime_error(
            "OP_RETURN data exceeds 80 bytes"
        );
    }

    if (script.size() != data_offset + data_size) {
        throw std::runtime_error(
            "invalid OP_RETURN data length"
        );
    }

    return script.substr(
        data_offset,
        data_size
    );
}

} // namespace larb
