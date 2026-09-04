#include "src/transaction.h"
#include "src/transaction_validator.h"
#include "src/consensus/constants.h"

#include <iostream>

int main() {
    std::cout << "=== LARB COINBASE TEST ===\n";

    const std::int64_t reward0 =
        larb::get_block_reward(0);

    larb::Transaction cb0 =
        larb::Transaction::coinbase(
            reward0,
            "larb1qmx5lm74z7jdzghxkr9g8n2pxdg7nuswckhp63pn04sws97vjtx0qh5ksjn"
        );

    if (!larb::validate_coinbase(cb0, reward0)) {
        std::cerr << "Block 0 coinbase: FAIL\n";
        return 1;
    }

    std::cout << "Block 0 coinbase: OK\n";

    const std::int64_t reward1 =
        larb::get_block_reward(210000);

    larb::Transaction cb1 =
        larb::Transaction::coinbase(
            reward1,
            "larb1qmx5lm74z7jdzghxkr9g8n2pxdg7nuswckhp63pn04sws97vjtx0qh5ksjn"
        );

    if (!larb::validate_coinbase(cb1, reward1)) {
        std::cerr << "Block 210000 coinbase: FAIL\n";
        return 1;
    }

    std::cout << "Block 210000 coinbase: OK\n";

    const std::int64_t reward_last =
        larb::get_block_reward(6720000);

    larb::Transaction cb_last =
        larb::Transaction::coinbase(
            reward_last,
            "larb1qmx5lm74z7jdzghxkr9g8n2pxdg7nuswckhp63pn04sws97vjtx0qh5ksjn"
        );

    if (!larb::validate_coinbase(
            cb_last,
            reward_last)) {
        std::cerr << "1 satoshi coinbase: FAIL\n";
        return 1;
    }

    std::cout << "1 satoshi coinbase: OK\n";

    const std::int64_t reward_zero =
        larb::get_block_reward(6930000);

    larb::Transaction cb_zero =
        larb::Transaction::coinbase(
            reward_zero,
            "larb1qmx5lm74z7jdzghxkr9g8n2pxdg7nuswckhp63pn04sws97vjtx0qh5ksjn"
        );

    if (larb::validate_coinbase(
            cb_zero,
            reward_zero)) {
        std::cerr << "Zero reward rejection: FAIL\n";
        return 1;
    }

    std::cout << "Zero reward rejection: OK\n";

    std::cout
        << "=== ALL COINBASE TESTS PASSED ===\n";

    return 0;
}
