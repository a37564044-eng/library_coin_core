#pragma once

#include <cstddef>
#include <string>

namespace larb {

class Script {
public:
    // Kapasitas data OP_RETURN LARB.
    static constexpr std::size_t OP_RETURN_MAX_DATA = 80;

    static std::string op_return(
        const std::string& data
    );

    static std::string pay_to_pubkey_hash(
        const std::string& public_key
    );

    static bool is_pay_to_pubkey_hash(
        const std::string& script
    );

    static std::string extract_pubkey_hash(
        const std::string& script
    );

    static bool is_op_return(
        const std::string& script
    );

    static std::string extract_op_return_data(
        const std::string& script
    );
};

} // namespace larb
