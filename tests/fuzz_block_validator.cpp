#include "../src/block_validator.h"
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main() {
    std::mt19937 rng(24680);

    for (int i = 0; i < 100000; ++i) {
        std::vector<std::string> txs;
        const std::size_t count = rng() % 8;

        for (std::size_t j = 0; j < count; ++j) {
            const std::size_t size = rng() % 256;
            std::string tx(size, '\0');

            for (char& c : tx)
                c = static_cast<char>(rng() & 0xff);

            txs.push_back(std::move(tx));
        }

        larb::Block block(
            static_cast<std::uint32_t>(rng()),
            std::string(rng() % 80, 'x'),
            txs,
            rng(),
            rng()
        );

        larb::UTXOSet utxos;

        try {
            (void)larb::validate_block(block, 1, utxos);
        } catch (...) {
            std::cerr << "UNCAUGHT EXCEPTION at iteration "
                      << i << "\n";
            return 1;
        }
    }

    std::cout << "Fuzz block validation: PASS\n";
    return 0;
}
