#include "src/transaction.h"
#include "src/script.h"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB Transaction Test ==="
              << std::endl;

    larb::Transaction tx;

    tx.version = 1;

    tx.inputs.push_back({
        "0000000000000000000000000000000000000000000000000000000000000000",
        0
    });

    tx.outputs.push_back(
        larb::TransactionOutput::op_return("LARB")
    );

    assert(tx.outputs.size() == 1);

    const auto& output = tx.outputs[0];

    assert(output.amount == 0);

    assert(
        larb::Script::is_op_return(
            output.script_pubkey
        )
    );

    const std::string recovered =
        larb::Script::extract_op_return_data(
            output.script_pubkey
        );

    assert(recovered == "LARB");

    std::cout << "Transaction created: OK"
              << std::endl;

    std::cout << "OP_RETURN output: OK"
              << std::endl;

    std::cout << "OP_RETURN data: "
              << recovered
              << std::endl;

    const std::string serialized =
        tx.serialize();

    assert(!serialized.empty());

    std::cout << "Serialization: OK"
              << std::endl;

    const std::string txid1 = tx.txid();
    const std::string txid2 = tx.txid();

    assert(txid1 == txid2);
    assert(txid1.size() == 64);

    std::cout << "TXID deterministic: OK"
              << std::endl;

    std::cout << "Transaction TXID: "
              << txid1
              << std::endl;

    std::cout << "=== ALL TRANSACTION TESTS PASSED ==="
              << std::endl;

    return 0;
}
