#include "src/transaction.h"
#include "src/script.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB OP_RETURN ROUNDTRIP TEST ===\n";

    const std::string data = "LARB-BLOG:HELLO";

    larb::Transaction tx{};
    tx.version = 1;

    tx.outputs.push_back(
        larb::TransactionOutput::op_return(data)
    );

    const std::string serialized = tx.serialize();

    const larb::Transaction decoded =
        larb::Transaction::deserialize(serialized);

    if (decoded.outputs.size() != 1) {
        std::cerr << "Output count: FAIL\n";
        return 1;
    }

    const auto& script =
        decoded.outputs[0].script_pubkey;

    if (!larb::Script::is_op_return(script)) {
        std::cerr << "OP_RETURN detection: FAIL\n";
        return 1;
    }

    const std::string extracted =
        larb::Script::extract_op_return_data(script);

    if (extracted != data) {
        std::cerr << "Data roundtrip: FAIL\n";
        return 1;
    }

    std::cout << "OP_RETURN detection: OK\n";
    std::cout << "Data roundtrip: OK\n";
    std::cout << "Data size: "
              << extracted.size()
              << " bytes\n";

    return 0;
}
