#include "src/transaction_validator.h"
#include "src/script.h"
#include "src/crypto/pqc.h"

#include <iostream>

int main() {
    using namespace larb;

    std::cout << "=== LARB TRANSACTION VALIDATOR TEST ===\n";

    auto keys = PQC::generate_keypair();

    UTXOSet utxos;

    Transaction funding{};
    funding.version = 1;
    funding.outputs.push_back({
        1000,
        Script::pay_to_pubkey_hash(keys.public_key)
    });

    const std::string funding_txid = funding.txid();

    utxos.add({
        OutPoint{funding_txid, 0},
        funding.outputs[0]
    });

    Transaction spend{};
    spend.version = 1;

    spend.inputs.push_back({
        funding_txid,
        0,
        "",
        ""
    });

    spend.outputs.push_back({
        900,
        "LARB-DEST"
    });

    if (!spend.sign_input(
            0,
            keys.public_key,
            keys.private_key)) {
        std::cerr << "Signing: FAIL\n";
        return 1;
    }

    if (!validate_transaction_inputs(spend, utxos)) {
        std::cerr << "Valid input: FAIL\n";
        return 1;
    }

    std::cout << "Valid P2PKH input: OK\n";
    std::cout << "Ownership + signature: OK\n";

    Transaction invalid{};
    invalid.version = 1;

    invalid.inputs.push_back({
        "nonexistent-txid",
        0,
        keys.public_key,
        ""
    });

    invalid.outputs.push_back({
        900,
        "LARB-DEST"
    });

    if (validate_transaction_inputs(invalid, utxos)) {
        std::cerr << "Missing UTXO rejection: FAIL\n";
        return 1;
    }

    std::cout << "Missing UTXO rejection: OK\n";

    std::cout
        << "=== ALL TRANSACTION VALIDATOR TESTS PASSED ===\n";

    return 0;
}
