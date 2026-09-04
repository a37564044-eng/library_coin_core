#include "../src/p2p.h"
#include <iostream>
#include <random>
#include <string>

int main() {
    std::mt19937 rng(13579);
    larb::P2PServer server(0);

    for (int i = 0; i < 100000; ++i) {
        const std::size_t size = rng() % 4096;
        std::string message(size, '\0');

        for (char& c : message)
            c = static_cast<char>(rng() & 0xff);

        if (i % 3 == 0)
            message = "SUBMIT_BLOCK\n" + message;

        std::string response;

        try {
            (void)server.handle_message(message, response);
        } catch (...) {
            std::cerr << "UNCAUGHT EXCEPTION at iteration "
                      << i << "\n";
            return 1;
        }
    }

    std::cout << "Fuzz P2P malformed packets: PASS\n";
    return 0;
}
