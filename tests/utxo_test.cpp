#include "src/utxo.h"

#include <cassert>
#include <iostream>

int main() {
    std::cout << "=== LARB UTXO Test ==="
              << std::endl;

    larb::OutPoint point_a{
        "5070e6a031c9d3631042e79929cb64e14a5fb1fe240cbb1077e44aa9a26d4198",
        0
    };

    larb::OutPoint point_b{
        "5070e6a031c9d3631042e79929cb64e14a5fb1fe240cbb1077e44aa9a26d4198",
        0
    };

    assert(point_a == point_b);

    std::cout << "OutPoint equality: OK"
              << std::endl;

    larb::TransactionOutput output{
        50'000'000,
        ""
    };

    larb::UTXO utxo{
        point_a,
        output
    };

    assert(utxo.outpoint.txid == point_a.txid);
    assert(utxo.outpoint.output_index == 0);
    assert(utxo.output.amount == 50'000'000);

    std::cout << "UTXO creation: OK"
              << std::endl;

    std::cout << "UTXO amount: "
              << utxo.output.amount
              << std::endl;

    std::cout << "=== ALL UTXO TESTS PASSED ==="
              << std::endl;

    return 0;
}
