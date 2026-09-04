#include "src/transaction_validator.h"

#include <iostream>

int main() {
    std::cout << "=== LARB TRANSACTION VALUE TEST ===\n";

    larb::Transaction funding{};
    funding.version = 1;
    funding.outputs.push_back({
        1000,
        "LARB-USER"
    });

    larb::UTXOSet utxos;

    larb::OutPoint point{
        funding.txid(),
        0
    };

    utxos.add({
        point,
        funding.outputs[0]
    });

    larb::Transaction valid{};
    valid.version = 1;
    valid.inputs.push_back({
        funding.txid(),
        0
    });
    valid.outputs.push_back({
        900,
        "LARB-DEST"
    });

    if (!larb::validate_transaction_value(valid, utxos)) {
        std::cerr << "1000 -> 900: FAIL\n";
        return 1;
    }

    std::cout << "1000 -> 900: OK\n";

    larb::Transaction invalid{};
    invalid.version = 1;
    invalid.inputs.push_back({
        funding.txid(),
        0
    });
    invalid.outputs.push_back({
        1100,
        "LARB-DEST"
    });

    if (larb::validate_transaction_value(invalid, utxos)) {
        std::cerr << "1000 -> 1100 rejection: FAIL\n";
        return 1;
    }

    std::cout << "1000 -> 1100 rejection: OK\n";

    std::cout << "=== ALL TRANSACTION VALUE TESTS PASSED ===\n";

    return 0;
}
