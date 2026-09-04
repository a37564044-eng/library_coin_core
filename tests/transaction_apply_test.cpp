#include "src/transaction_validator.h"

#include <iostream>

int main() {
    std::cout << "=== LARB TRANSACTION APPLY TEST ===\n";

    larb::Transaction funding{};
    funding.version = 1;
    funding.outputs.push_back({
        1000,
        "LARB-ALICE"
    });

    const std::string funding_txid = funding.txid();

    larb::UTXOSet utxos;

    utxos.add({
        {funding_txid, 0},
        funding.outputs[0]
    });

    larb::Transaction spend{};
    spend.version = 1;
    spend.inputs.push_back({
        funding_txid,
        0
    });

    spend.outputs.push_back({
        900,
        "LARB-BOB"
    });

    if (!larb::apply_transaction(spend, utxos)) {
        std::cerr << "Apply transaction: FAIL\n";
        return 1;
    }

    std::cout << "Apply transaction: OK\n";

    if (utxos.exists({funding_txid, 0})) {
        std::cerr << "Old UTXO spent: FAIL\n";
        return 1;
    }

    std::cout << "Old UTXO spent: OK\n";

    const std::string spend_txid = spend.txid();

    const larb::UTXO* new_utxo =
        utxos.find({spend_txid, 0});

    if (new_utxo == nullptr) {
        std::cerr << "New UTXO created: FAIL\n";
        return 1;
    }

    if (new_utxo->output.amount != 900) {
        std::cerr << "New UTXO value: FAIL\n";
        return 1;
    }

    std::cout << "New UTXO created: OK\n";
    std::cout << "New UTXO value: OK\n";

    std::cout
        << "=== ALL TRANSACTION APPLY TESTS PASSED ===\n";

    return 0;
}
