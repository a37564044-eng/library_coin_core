#include "transaction.h"

#include <iostream>

int main() {
    std::cout << "=== LARB TRANSACTION CODEC TEST ===\n";

    larb::Transaction original =
        larb::Transaction::coinbase(
            5'000'000'000,
            "LARB-MINER"
        );

    const std::string encoded =
        original.serialize();

    larb::Transaction decoded =
        larb::Transaction::deserialize(encoded);

    if (decoded.version != original.version) {
        std::cerr << "Version: FAIL\n";
        return 1;
    }

    if (decoded.inputs.size() != original.inputs.size()) {
        std::cerr << "Inputs: FAIL\n";
        return 1;
    }

    if (decoded.outputs.size() != original.outputs.size()) {
        std::cerr << "Outputs: FAIL\n";
        return 1;
    }

    if (decoded.outputs[0].amount !=
        original.outputs[0].amount) {
        std::cerr << "Amount: FAIL\n";
        return 1;
    }

    if (decoded.outputs[0].script_pubkey !=
        original.outputs[0].script_pubkey) {
        std::cerr << "Script: FAIL\n";
        return 1;
    }

    if (decoded.txid() != original.txid()) {
        std::cerr << "TXID: FAIL\n";
        return 1;
    }

    std::cout << "Serialize: OK\n";
    std::cout << "Deserialize: OK\n";
    std::cout << "TXID preserved: OK\n";
    std::cout
        << "=== ALL TRANSACTION CODEC TESTS PASSED ===\n";

    return 0;
}
