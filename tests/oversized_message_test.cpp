#include "../src/p2p.h"
#include <iostream>
#include <string>

int main() {
    larb::P2PServer server(0);

    const std::size_t sizes[] = {
        1024 * 1024,
        4 * 1024 * 1024,
        16 * 1024 * 1024
    };

    for (std::size_t size : sizes) {
        std::string message(size, 'X');
        std::string response;

        try {
            (void)server.handle_message(message, response);
        } catch (...) {
            std::cerr << "Oversized message crashed at "
                      << size << " bytes\n";
            return 1;
        }
    }

    std::cout << "Oversized messages: PASS\n";
    return 0;
}
