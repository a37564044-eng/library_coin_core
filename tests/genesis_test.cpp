#include "src/genesis.h"
#include "src/script.h"
#include "src/transaction.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB GENESIS TEST ===\n";

    const larb::Block genesis = larb::Genesis::create();

    if (genesis.header().previous_hash != "0") {
        std::cerr << "Genesis previous hash: FAIL\n";
        return 1;
    }

    std::cout << "Genesis previous hash: OK\n";

    if (genesis.header().timestamp != larb::Genesis::TIMESTAMP) {
        std::cerr << "Genesis timestamp: FAIL\n";
        return 1;
    }

    std::cout << "Genesis timestamp: OK\n";

    if (genesis.header().nonce != larb::Genesis::NONCE) {
        std::cerr << "Genesis nonce: FAIL\n";
        return 1;
    }

    std::cout << "Genesis nonce: OK\n";

    if (genesis.transactions().size() != 1) {
        std::cerr << "Genesis transaction count: FAIL\n";
        return 1;
    }

    std::cout << "Genesis transaction count: OK\n";

    const larb::Transaction tx =
        larb::Transaction::deserialize(
            genesis.transactions()[0]
        );

    if (tx.outputs.size() != 1) {
        std::cerr << "Genesis output count: FAIL\n";
        return 1;
    }

    std::cout << "Genesis output count: OK\n";

    const std::string message =
        larb::Script::extract_op_return_data(
            tx.outputs[0].script_pubkey
        );

    if (message != larb::Genesis::MESSAGE) {
        std::cerr << "Genesis message: FAIL\n";
        return 1;
    }

    std::cout << "Genesis message: OK\n";

    const std::string hash1 = genesis.hash();
    const std::string hash2 = genesis.hash();

    if (hash1 != hash2) {
        std::cerr << "Genesis hash deterministic: FAIL\n";
        return 1;
    }

    std::cout << "Genesis hash deterministic: OK\n";
    std::cout << "Genesis hash: " << hash1 << "\n";

    std::cout << "=== ALL GENESIS TESTS PASSED ===\n";

    return 0;
}
