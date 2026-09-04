#include "src/p2p.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB P2P TEST ===\n";

    larb::P2PServer server(18444);

    std::string response;

    if (!server.handle_message("PING", response)) {
        std::cerr << "PING handling: FAIL\n";
        return 1;
    }

    std::cout << "PING handling: OK\n";

    if (response != "PONG") {
        std::cerr << "PONG response: FAIL\n";
        return 1;
    }

    std::cout << "PONG response: OK\n";

    response.clear();

    if (server.handle_message("UNKNOWN", response)) {
        std::cerr << "Unknown message rejection: FAIL\n";
        return 1;
    }

    if (response != "ERROR") {
        std::cerr << "ERROR response: FAIL\n";
        return 1;
    }

    std::cout << "Unknown message rejection: OK\n";

    std::cout << "=== ALL P2P TESTS PASSED ===\n";

    return 0;
}
