#include "src/transaction.h"
#include "src/crypto/pqc.h"
#include <cassert>
#include <iostream>

int main() {
    auto keys = larb::PQC::generate_keypair();

    larb::Transaction tx;
    tx.version = 1;
    tx.inputs.push_back({
        "0000000000000000000000000000000000000000000000000000000000000000",
        0, "", ""
    });
    tx.outputs.push_back(
        larb::TransactionOutput::op_return("LARB")
    );

    assert(tx.sign_input(0, keys.public_key, keys.private_key));
    assert(tx.verify_input(0));

    tx.outputs[0] =
        larb::TransactionOutput::op_return("TAMPERED");

    assert(!tx.verify_input(0));

    std::cout << "=== TRANSACTION SIGNATURE: PASS ===\n";
    return 0;
}
