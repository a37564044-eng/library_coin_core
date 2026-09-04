#include "../src/transaction_validator.h"
#include <iostream>
#include <random>

int main() {
    std::mt19937 rng(67890);
    larb::UTXOSet utxos;

    for (int i = 0; i < 100000; ++i) {
        larb::Transaction tx;

        const std::size_t inputs = rng() % 8;
        const std::size_t outputs = rng() % 8;

        for (std::size_t j = 0; j < inputs; ++j) {
            larb::TransactionInput input;
            input.previous_txid.resize(rng() % 80);
            input.output_index = rng();
            input.public_key.resize(rng() % 80);
            input.signature.resize(rng() % 160);
            tx.inputs.push_back(std::move(input));
        }

        for (std::size_t j = 0; j < outputs; ++j) {
            larb::TransactionOutput output;
            output.amount = static_cast<std::int64_t>(rng());
            output.script_pubkey.resize(rng() % 100);
            tx.outputs.push_back(std::move(output));
        }

        try {
            (void)larb::validate_transaction_inputs(tx, utxos);
            (void)larb::validate_transaction_value(tx, utxos);
        } catch (...) {
            std::cerr << "UNCAUGHT EXCEPTION at iteration "
                      << i << "\n";
            return 1;
        }
    }

    std::cout << "Fuzz transaction validation: PASS\n";
    return 0;
}
