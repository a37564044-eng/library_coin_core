#include "src/transaction.h"
#include "src/transaction_validator.h"
#include "src/utxo_set.h"
#include "src/consensus/constants.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB COINBASE ADDRESS TEST ===\n";

    /*
     * F4.1 canonical address format:
     *     larb1q...
     *
     * Coinbase harus menyimpan address ini
     * sebagai script_pubkey tanpa mengubah payload.
     */
    const std::string address =
        "larb1qmx5lm74z7jdzghxkr9g8n2pxdg7nuswckhp63pn04sws97vjtx0qh5ksjn";

    if (address.rfind("larb1q", 0) != 0) {
        std::cerr << "Address format: FAIL\n";
        return 1;
    }

    const std::int64_t reward =
        larb::get_block_reward(1);

    larb::Transaction coinbase =
        larb::Transaction::coinbase(
            reward,
            address
        );

    larb::UTXOSet utxos;

    if (!larb::apply_coinbase(
            coinbase,
            reward,
            utxos)) {
        std::cerr << "Apply coinbase: FAIL\n";
        return 1;
    }

    const auto* result =
        utxos.find({
            coinbase.txid(),
            0
        });

    if (result == nullptr) {
        std::cerr << "Reward UTXO: FAIL\n";
        return 1;
    }

    if (result->output.script_pubkey != address) {
        std::cerr << "Reward address: FAIL\n";
        return 1;
    }

    std::cout << "Address format: OK\n";
    std::cout << "Coinbase accepted: OK\n";
    std::cout << "Reward UTXO created: OK\n";
    std::cout << "Reward address preserved: OK\n";
    std::cout << "Address: "
              << result->output.script_pubkey
              << "\n";

    std::cout
        << "=== COINBASE ADDRESS TEST PASSED ===\n";

    return 0;
}
