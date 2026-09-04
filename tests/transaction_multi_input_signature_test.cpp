#include "src/transaction.h"
#include "src/script.h"
#include "src/crypto/pqc.h"

#include <cassert>
#include <iostream>

int main() {
    using namespace larb;

    std::cout << "=== LARB MULTI-INPUT SIGNATURE TEST ===\n";

    auto keys1 = PQC::generate_keypair();
    auto keys2 = PQC::generate_keypair();

    Transaction tx;
    tx.version = 1;

    tx.inputs.push_back({
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        0,
        "",
        ""
    });

    tx.inputs.push_back({
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
        1,
        "",
        ""
    });

    tx.outputs.push_back({
        2500,
        "recipient"
    });

    assert(tx.sign_input(
        0,
        keys1.public_key,
        keys1.private_key
    ));

    assert(tx.sign_input(
        1,
        keys2.public_key,
        keys2.private_key
    ));

    assert(tx.verify_input(0));
    assert(tx.verify_input(1));

    std::cout << "Two signatures valid: OK\n";

    /*
     * Mengubah output harus membatalkan
     * signature untuk kedua input.
     */
    tx.outputs[0].amount = 2499;

    assert(!tx.verify_input(0));
    assert(!tx.verify_input(1));

    std::cout << "Transaction tampering detected: OK\n";

    /*
     * Kembalikan output.
     */
    tx.outputs[0].amount = 2500;

    /*
     * Signature input 0 tidak boleh
     * digantikan dengan signature input 1.
     */
    assert(tx.sign_input(
        0,
        keys1.public_key,
        keys1.private_key
    ));

    tx.inputs[0].signature =
        tx.inputs[1].signature;

    assert(!tx.verify_input(0));
    assert(tx.verify_input(1));

    std::cout << "Signature swap rejection: OK\n";

    std::cout
        << "=== ALL MULTI-INPUT SIGNATURE TESTS PASSED ===\n";

    return 0;
}
