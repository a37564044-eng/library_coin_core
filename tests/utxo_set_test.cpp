#include "src/utxo_set.h"

#include <cassert>
#include <iostream>

int main() {
    std::cout << "=== LARB UTXO SET TEST ==="
              << std::endl;

    larb::UTXOSet set;

    larb::UTXO utxo{
        {
            "5070e6a031c9d3631042e79929cb64e14a5fb1fe240cbb1077e44aa9a26d4198",
            0
        },
        {
            50'000'000,
            ""
        }
    };

    set.add(utxo);

    assert(set.size() == 1);

    std::cout << "UTXO add: OK"
              << std::endl;

    assert(set.exists(utxo.outpoint));

    std::cout << "UTXO find: OK"
              << std::endl;

    const larb::UTXO* found =
        set.find(utxo.outpoint);

    assert(found != nullptr);
    assert(found->output.amount == 50'000'000);

    std::cout << "UTXO amount check: OK"
              << std::endl;

    assert(set.spend(utxo.outpoint));

    std::cout << "UTXO spend: OK"
              << std::endl;

    assert(!set.exists(utxo.outpoint));
    assert(set.size() == 0);

    std::cout << "UTXO removal: OK"
              << std::endl;

    std::cout << "=== ALL UTXO SET TESTS PASSED ==="
              << std::endl;

    return 0;
}
