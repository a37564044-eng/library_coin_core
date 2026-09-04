#include "transaction_validator.h"
#include "consensus/constants.h"
#include "transaction.h"
#include "utxo_set.h"

#include <iostream>

int main() {
    std::cout << "=== LARB MONEY RANGE TEST ===\n";

    larb::UTXOSet utxos;

    larb::Transaction tx;
    tx.version = 1;
    tx.inputs.push_back({
        "funding",
        0
    });

    utxos.add({
        {"funding", 0},
        {larb::MAX_MONEY, "OWNER"}
    });

    tx.outputs = {
        {-1, "A"}
    };

    if (larb::validate_transaction_value(tx, utxos)) {
        std::cerr << "Negative output rejection: FAIL\n";
        return 1;
    }

    std::cout << "Negative output rejection: OK\n";

    tx.outputs = {
        {larb::MAX_MONEY + 1, "A"}
    };

    if (larb::validate_transaction_value(tx, utxos)) {
        std::cerr << "MAX_MONEY + 1 rejection: FAIL\n";
        return 1;
    }

    std::cout << "MAX_MONEY + 1 rejection: OK\n";

    tx.outputs = {
        {larb::MAX_MONEY, "A"}
    };

    if (!larb::validate_transaction_value(tx, utxos)) {
        std::cerr << "MAX_MONEY accepted: FAIL\n";
        return 1;
    }

    std::cout << "MAX_MONEY accepted: OK\n";

    tx.outputs = {
        {larb::MAX_MONEY - 1, "A"}
    };

    if (!larb::validate_transaction_value(tx, utxos)) {
        std::cerr << "Below MAX_MONEY: FAIL\n";
        return 1;
    }

    std::cout << "Below MAX_MONEY: OK\n";
    std::cout << "=== ALL MONEY RANGE TESTS PASSED ===\n";

    return 0;
}
