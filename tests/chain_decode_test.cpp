#include "src/block.h"
#include "src/blockchain.h"
#include "src/chain_codec.h"
#include "src/genesis.h"
#include "src/genesis.h"

#include <iostream>

int main() {
    std::cout << "=== LARB CHAIN DECODE TEST ===\n";

    larb::Block genesis = larb::Genesis::create();

    larb::Blockchain chain(genesis, 0);

    larb::Block block1(
        1,
        genesis.hash(),
        {"transaction-1"},
        1700000001,
        1
    );

    if (!chain.add_block(block1)) {
        std::cerr << "Chain preparation: FAIL\n";
        return 1;
    }

    std::cout << "Chain preparation: OK\n";

    const std::string encoded =
        larb::serialize_chain(chain);

    std::cout << "Encode: OK\n";

    const auto decoded =
        larb::deserialize_chain(encoded);

    if (!decoded) {
        std::cerr << "Decode: FAIL\n";
        return 1;
    }

    std::cout << "Decode: OK\n";

    if (!decoded->is_valid()) {
        std::cerr << "Decoded validation: FAIL\n";
        return 1;
    }

    std::cout << "Decoded validation: OK\n";

    if (decoded->size() != chain.size()) {
        std::cerr << "Chain size: FAIL\n";
        return 1;
    }

    std::cout << "Chain size: OK\n";

    if (larb::serialize_chain(*decoded) != encoded) {
        std::cerr << "Round-trip: FAIL\n";
        return 1;
    }

    std::cout << "Round-trip: OK\n";

    std::cout << "=== ALL CHAIN DECODE TESTS PASSED ===\n";
    return 0;
}
