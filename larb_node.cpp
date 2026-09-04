#include "src/genesis.h"
#include "src/consensus/constants.h"
#include "src/node.h"
#include "src/p2p.h"

#include <atomic>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>

namespace {
std::atomic<bool> running{true};

void handle_signal(int) {
    running = false;
}
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    if (argc >= 2 &&
        (std::string(argv[1]) == "--help" ||
         std::string(argv[1]) == "-h")) {
        std::cout << "LARB Full Node\n";
        std::cout << "Usage: ./larb_node [PORT] [PEER_HOST] [PEER_PORT] [CHAIN_FILE]\n";
        std::cout << "\n";
        std::cout << "Defaults:\n";
        std::cout << "  PORT       8333\n";
        std::cout << "  CHAIN_FILE larb_chain.dat\n";
        return 0;
    }

    std::cout << "=== LARB FULL NODE ===\n";

    const larb::Block genesis = larb::Genesis::create();

    larb::Node node(genesis, larb::INITIAL_POW_DIFFICULTY);
    const bool loaded = node.load_state(argc >= 5 ? argv[4] : "larb_chain.dat");
    std::cout << (loaded ? "Persistence: LOADED\n" : "Persistence: NEW\n");

    std::cout << "Genesis: OK\n";
    std::cout << "Chain height: "
              << node.chain_size() - 1
              << '\n';

    std::uint16_t port = 8333;
    if (argc >= 2) {
        port = static_cast<std::uint16_t>(std::stoul(argv[1]));
    }

    larb::P2PServer server(port);

    server.set_blockchain(&node.blockchain());
    server.set_node(&node);

    if (!server.start()) {
        std::cerr << "P2P server start: FAILED\n";
        return 1;
    }

    std::cout << "P2P server: OK\n";
    std::cout << "P2P port: " << port << "\n";
    if (argc >= 4) {
        const std::uint16_t peer_port = static_cast<std::uint16_t>(std::stoul(argv[3]));
        server.add_peer(argv[2], peer_port);
        const bool synced = server.sync_from_peer(argv[2], peer_port);
        std::cout << (synced ? "Peer sync: OK\n" : "Peer sync: NO CHANGE\n");
        node.save_state(argc >= 5 ? argv[4] : "larb_chain.dat");
    }

    std::cout << "Peer count: "
              << server.peer_count()
              << '\n';

    std::cout << "Node running...\n";

    while (running) {
        server.serve_once();
    }

    server.stop();

    std::cout << "P2P server stopped.\n";
    std::cout << "=== LARB NODE STOPPED ===\n";

    return 0;
}
