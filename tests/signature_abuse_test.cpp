#include "transaction.h"
#include "crypto/pqc.h"
#include <cassert>
#include <iostream>

int main() {
    auto keys = larb::PQC::generate_keypair();
    auto other = larb::PQC::generate_keypair();

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

    tx.inputs[0].signature.clear();
    assert(!tx.verify_input(0));

    assert(tx.sign_input(0, keys.public_key, keys.private_key));
    tx.inputs[0].signature.resize(
        tx.inputs[0].signature.size() / 2
    );
    assert(!tx.verify_input(0));

    assert(tx.sign_input(0, keys.public_key, keys.private_key));
    tx.inputs[0].signature[0] ^= 0x01;
    assert(!tx.verify_input(0));

    assert(tx.sign_input(0, keys.public_key, keys.private_key));
    tx.inputs[0].public_key = other.public_key;
    assert(!tx.verify_input(0));

    assert(tx.sign_input(0, other.public_key, other.private_key));
    tx.inputs[0].public_key = keys.public_key;
    assert(!tx.verify_input(0));

    std::cout << "Signature abuse tests: PASS\n";
    return 0;
}
