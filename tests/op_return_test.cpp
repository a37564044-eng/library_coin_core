#include "src/script.h"
#include "src/transaction.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB OP_RETURN TEST ===\n";

    const std::string data = "LARB BLOG";

    const std::string script =
        larb::Script::op_return(data);

    if (!larb::Script::is_op_return(script)) {
        std::cerr << "OP_RETURN detect: FAIL\n";
        return 1;
    }

    if (larb::Script::extract_op_return_data(script) != data) {
        std::cerr << "OP_RETURN extract: FAIL\n";
        return 1;
    }

    std::cout << "OP_RETURN encode/decode: OK\n";

    const auto output =
        larb::TransactionOutput::op_return(data);

    if (!larb::Script::is_op_return(output.script_pubkey)) {
        std::cerr << "Transaction OP_RETURN: FAIL\n";
        return 1;
    }

    if (larb::Script::extract_op_return_data(
            output.script_pubkey) != data) {
        std::cerr << "Transaction OP_RETURN data: FAIL\n";
        return 1;
    }

    std::cout << "Transaction OP_RETURN: OK\n";

    const std::string data80(80, 'A');

    const std::string script80 =
        larb::Script::op_return(data80);

    if (larb::Script::extract_op_return_data(script80)
        != data80) {
        std::cerr << "80-byte OP_RETURN: FAIL\n";
        return 1;
    }

    std::cout << "80-byte limit: OK\n";

    try {
        larb::Script::op_return(std::string(81, 'B'));

        std::cerr << "81-byte rejection: FAIL\n";
        return 1;
    } catch (...) {
        std::cout << "81-byte rejection: OK\n";
    }

    std::cout
        << "=== ALL OP_RETURN TESTS PASSED ===\n";

    return 0;
}
