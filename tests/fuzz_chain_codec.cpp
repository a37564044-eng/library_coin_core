#include "../src/chain_codec.h"
#include <iostream>
#include <random>
#include <string>

int main() {
    std::mt19937 rng(12345);

    for (int i = 0; i < 100000; ++i) {
        const std::size_t size = rng() % 512;
        std::string data(size, '\0');

        for (char& c : data)
            c = static_cast<char>(rng() & 0xff);

        try {
            (void)larb::deserialize_chain(data);
        } catch (...) {
            std::cerr << "UNCAUGHT EXCEPTION at iteration "
                      << i << "\n";
            return 1;
        }
    }

    std::cout << "Fuzz serialization: PASS\n";
    return 0;
}
