#include "src/genesis.h"
#include "src/script.h"
#include "src/transaction.h"

#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    std::cout << "=== LARB GENESIS OP_RETURN TEST ===\n";

    const std::string message =
        "LARB GENESIS - FREEDOM OF INFORMATION";

    larb::Block genesis =
        larb::Genesis::create(
            message,
            1,
            0
        );

    if (genesis.header().previous_hash != "0") {
        std::cerr << "Previous hash: FAIL\n";
        return 1;
    }

    if (genesis.transactions().size() != 1) {
        std::cerr << "Genesis transaction count: FAIL\n";
        return 1;
    }

    const larb::Transaction tx =
        larb::Transaction::deserialize(
            genesis.transactions()[0]
        );

    if (tx.outputs.size() != 1) {
        std::cerr << "Genesis output count: FAIL\n";
        return 1;
    }

    const std::string& script =
        tx.outputs[0].script_pubkey;

    if (!larb::Script::is_op_return(script)) {
        std::cerr << "Genesis OP_RETURN: FAIL\n";
        return 1;
    }

    const std::string recovered =
        larb::Script::extract_op_return_data(script);

    if (recovered != message) {
        std::cerr << "Genesis message: FAIL\n";
        return 1;
    }

    std::cout << "Previous hash = 0: OK\n";
    std::cout << "Genesis transaction: OK\n";
    std::cout << "OP_RETURN: OK\n";
    std::cout << "Genesis message: [" << recovered << "]\n";
    std::cout << "TXID: " << tx.txid() << "\n";
    std::cout << "Genesis hash: " << genesis.hash() << "\n";
    std::cout << "=== GENESIS OP_RETURN TEST PASSED ===\n";

    return 0;
}
