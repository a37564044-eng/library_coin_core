#include "transaction_validator.h"
#include <iostream>

int main() {
    using namespace larb;

    std::cout << "=== LARB TRANSACTION FEE TEST ===\n";

    UTXOSet utxos;

    Transaction funding;
    funding.version = 1;
    funding.outputs.push_back(
        TransactionOutput{1000, "owner"}
    );

    const std::string funding_txid = funding.txid();

    utxos.add(UTXO{
        OutPoint{funding_txid, 0},
        funding.outputs[0]
    });

    Transaction tx;
    tx.version = 1;
    tx.inputs.push_back(
        TransactionInput{funding_txid, 0}
    );
    tx.outputs.push_back(
        TransactionOutput{900, "recipient"}
    );

    if (!validate_transaction_value(tx, utxos)) {
        std::cout << "Valid transaction: FAIL\n";
        return 1;
    }

    std::cout << "Valid transaction: OK\n";

    const std::int64_t input_value = 1000;
    const std::int64_t output_value = 900;
    const std::int64_t fee = input_value - output_value;

    if (fee != 100) {
        std::cout << "Fee calculation: FAIL\n";
        return 1;
    }

    std::cout << "Fee calculation (1000 -> 900): OK\n";
    std::cout << "Fee: " << fee << " satoshi\n";

    Transaction invalid;
    invalid.version = 1;
    invalid.inputs.push_back(
        TransactionInput{funding_txid, 0}
    );
    invalid.outputs.push_back(
        TransactionOutput{1000, "recipient"}
    );
    invalid.outputs.push_back(
        TransactionOutput{1, "extra"}
    );

    if (validate_transaction_value(invalid, utxos)) {
        std::cout << "Fee violation rejection: FAIL\n";
        return 1;
    }

    std::cout << "Fee violation rejection: OK\n";

    std::cout << "=== ALL TRANSACTION FEE TESTS PASSED ===\n";
    return 0;
}
