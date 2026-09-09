#include "src/consensus/constants.h"
#include "src/transaction.h"

#include <iostream>

int main() {
    std::cout << "=== TRANSACTION ROUNDTRIP DEBUG ===\n";

    const larb::Transaction original =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-MINER"
        );

    const std::string serialized =
        original.serialize();

    std::cout << "Serialized size: "
              << serialized.size()
              << '\n';

    std::cout << "Serialized: ["
              << serialized
              << "]\n";

    try {
        const larb::Transaction decoded =
            larb::Transaction::deserialize(
                serialized
            );

        std::cout << "Deserialize: OK\n";
        std::cout << "Decoded inputs: "
                  << decoded.inputs.size()
                  << '\n';

        std::cout << "Decoded outputs: "
                  << decoded.outputs.size()
                  << '\n';

        std::cout << "Original txid: "
                  << original.txid()
                  << '\n';

        std::cout << "Decoded txid: "
                  << decoded.txid()
                  << '\n';

        std::cout << "TXID equal: "
                  << (original.txid() == decoded.txid()
                      ? "YES"
                      : "NO")
                  << '\n';

    } catch (const std::exception& e) {
        std::cout << "Deserialize: FAIL\n";
        std::cout << "Exception: "
                  << e.what()
                  << '\n';
        return 1;
    }

    return 0;
}
