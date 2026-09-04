#include "block.h"
#include "block_validator.h"
#include "consensus/constants.h"
#include "utxo_set.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

int main() {
    std::cout << "=== LARB BLOCK TIMESTAMP TEST ===\n";

    larb::UTXOSet utxos;

    /*
     * Timestamp UINT64_MAX sengaja dibuat sangat jauh
     * ke masa depan.
     *
     * Untuk sementara test ini menguji bahwa validator
     * memang memiliki aturan timestamp.
     */

    larb::Block future_block(
        1,
        "0",
        {"timestamp-test"},
        std::numeric_limits<std::uint64_t>::max(),
        0
    );

    const bool accepted =
        larb::validate_block(
            future_block,
            0,
            utxos
        );

    if (accepted) {
        std::cerr << "Future timestamp rejection: FAIL\n";
        return 1;
    }

    std::cout << "Future timestamp rejection: OK\n";
    std::cout << "=== ALL BLOCK TIMESTAMP TESTS PASSED ===\n";

    return 0;
}
