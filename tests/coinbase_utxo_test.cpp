#include "src/transaction.h"
#include "src/transaction_validator.h"
#include "src/consensus/constants.h"

#include <iostream>

int main() {
    std::cout << "=== LARB COINBASE UTXO TEST ===\n";

    const std::int64_t reward =
        larb::get_block_reward(0);

    larb::Transaction coinbase =
        larb::Transaction::coinbase(
            reward,
            "larb1qmx5lm74z7jdzghxkr9g8n2pxdg7nuswckhp63pn04sws97vjtx0qh5ksjn"
        );

    larb::UTXOSet utxos;

    if (!larb::apply_coinbase(
            coinbase,
            reward,
            utxos)) {
        std::cerr << "Apply coinbase: FAIL\n";
        return 1;
    }

    std::cout << "Apply coinbase: OK\n";

    const std::string txid = coinbase.txid();

    const larb::UTXO* result =
        utxos.find({txid, 0});

    if (result == nullptr) {
        std::cerr << "Reward UTXO created: FAIL\n";
        return 1;
    }

    std::cout << "Reward UTXO created: OK\n";

    if (result->output.amount != reward) {
        std::cerr << "Reward amount: FAIL\n";
        return 1;
    }

    std::cout << "Reward amount: OK\n";

    if (result->output.script_pubkey != "larb1qmx5lm74z7jdzghxkr9g8n2pxdg7nuswckhp63pn04sws97vjtx0qh5ksjn") {
        std::cerr << "Reward owner: FAIL\n";
        return 1;
    }

    std::cout << "Reward owner: OK\n";

    if (utxos.size() != 1) {
        std::cerr << "UTXO set size: FAIL\n";
        return 1;
    }

    std::cout << "UTXO set size: OK\n";

    std::cout
        << "=== ALL COINBASE UTXO TESTS PASSED ===\n";

    return 0;
}
