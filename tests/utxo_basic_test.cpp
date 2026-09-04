#include "src/utxo_set.h"

#include <iostream>

int main() {
    std::cout << "=== LARB UTXO BASIC TEST ===\n";

    larb::UTXOSet set;

    larb::UTXO utxo{
        {"txid-test-1", 0},
        {5'000'000'000, "LARB_ADDRESS"}
    };

    set.add(utxo);

    larb::OutPoint point{"txid-test-1", 0};

    if (!set.exists(point)) {
        std::cerr << "Add/exists: FAIL\n";
        return 1;
    }

    std::cout << "Add/exists: OK\n";

    const larb::UTXO* found = set.find(point);

    if (found == nullptr ||
        found->output.amount != 5'000'000'000) {
        std::cerr << "Find: FAIL\n";
        return 1;
    }

    std::cout << "Find: OK\n";

    if (!set.spend(point)) {
        std::cerr << "Spend: FAIL\n";
        return 1;
    }

    if (set.exists(point)) {
        std::cerr << "Spent UTXO still exists: FAIL\n";
        return 1;
    }

    std::cout << "Spend: OK\n";

    if (set.size() != 0) {
        std::cerr << "Final size: FAIL\n";
        return 1;
    }

    std::cout << "Final size: OK\n";
    std::cout << "=== ALL UTXO BASIC TESTS PASSED ===\n";

    return 0;
}
