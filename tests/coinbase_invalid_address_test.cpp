#include "src/transaction.h"
#include "src/transaction_validator.h"
#include "src/consensus/constants.h"

#include <iostream>

int main() {
    std::cout << "=== LARB F4.2 INVALID COINBASE ADDRESS TEST ===\n";

    const std::int64_t reward =
        larb::get_block_reward(1);

    // Address palsu / bukan Bech32 LARB.
    larb::Transaction bad1 =
        larb::Transaction::coinbase(
            reward,
            "LARB6ad3258d55df72ca79cd42b1fc12f6cea161c49866705981c7b477ba18dfba9a"
        );

    if (larb::validate_coinbase(bad1, reward)) {
        std::cerr << "Invalid legacy address accepted: FAIL\n";
        return 1;
    }

    // Address dengan HRP salah.
    larb::Transaction bad2 =
        larb::Transaction::coinbase(
            reward,
            "test1qqqqsyqcyq5rqwzqfpg9scrgwpugpzysnzs23v9ccrydpk8qarc0spev0cw"
        );

    if (larb::validate_coinbase(bad2, reward)) {
        std::cerr << "Wrong HRP accepted: FAIL\n";
        return 1;
    }

    // Address valid dari F4.1.
    larb::Transaction good =
        larb::Transaction::coinbase(
            reward,
            "larb1qqqqsyqcyq5rqwzqfpg9scrgwpugpzysnzs23v9ccrydpk8qarc0spev0cw"
        );

    if (!larb::validate_coinbase(good, reward)) {
        std::cerr << "Valid LARB address rejected: FAIL\n";
        return 1;
    }

    std::cout << "Invalid legacy address rejected: OK\n";
    std::cout << "Wrong HRP rejected: OK\n";
    std::cout << "Valid LARB address accepted: OK\n";
    std::cout << "=== F4.2 COINBASE ADDRESS TEST PASSED ===\n";

    return 0;
}
