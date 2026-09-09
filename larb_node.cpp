#include "src/genesis.h"
#include "src/consensus/constants.h"
#include "src/node.h"
#include "src/p2p.h"
#include "src/consensus/pow.h"
#include "src/transaction.h"
#include "src/wallet/wallet.h"

#include <atomic>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

namespace {
std::atomic<bool> running{true};

larb::Node* active_node = nullptr;
larb::P2PServer* active_server = nullptr;
std::mutex active_mutex;

void handle_signal(int) {
    running = false;
}
}

int run_node(int argc, char* argv[]) {
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

    {
        std::lock_guard<std::mutex> lock(active_mutex);
        active_node = &node;
        active_server = &server;
    }

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

    {
        std::lock_guard<std::mutex> lock(active_mutex);
        active_node = nullptr;
        active_server = nullptr;
    }

    std::cout << "P2P server stopped.\n";
    std::cout << "=== LARB NODE STOPPED ===\n";

    return 0;
}


extern "C" int larb_run_node(int argc, char* argv[]) {
    running = true;
    return run_node(argc, argv);
}

extern "C" void larb_stop_node() {
    running = false;
}

extern "C" const char* larb_mine_once(
    const larb::Wallet& wallet
) {
    static std::string result;

    std::lock_guard<std::mutex> lock(active_mutex);

    if (!active_node) {
        result = "ERROR: NODE NOT RUNNING";
        return result.c_str();
    }

    try {
        larb::Node* node = active_node;
        const auto& blockchain = node->blockchain();

        const std::size_t height = blockchain.size();

        const std::int64_t reward =
            larb::get_block_reward(height);

        if (reward <= 0) {
            result = "ERROR: BLOCK REWARD EXHAUSTED";
            return result.c_str();
        }

        const larb::Transaction coinbase =
            larb::Transaction::coinbase(
                reward,
                wallet.address(),
                height
            );

        const std::string coinbase_data =
            coinbase.serialize();

        const std::uint64_t timestamp =
            static_cast<std::uint64_t>(
                std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now()
                )
            );

        const std::string previous_hash =
            blockchain.at(height - 1).hash();

        const larb::Block block =
            larb::mine_block(
                1,
                previous_hash,
                {coinbase_data},
                timestamp,
                blockchain.difficulty()
            );

        if (!larb::validate_proof_of_work(
                block,
                blockchain.difficulty())) {
            result = "ERROR: POW VALIDATION FAILED";
            return result.c_str();
        }

        if (!node->receive_block(block)) {
            result = "ERROR: BLOCK REJECTED";
            return result.c_str();
        }

        if (!node->save_state("larb_chain.dat")) {
            result = "ERROR: BLOCKCHAIN SAVE FAILED";
            return result.c_str();
        }

        result =
            "MINED OK\n"
            "HEIGHT: " + std::to_string(height) + "\n"
            "REWARD: " + std::to_string(reward) + "\n"
            "NONCE: " + std::to_string(block.header().nonce) + "\n"
            "HASH: " + block.hash() + "\n"
            "TXID: " + coinbase.txid() + "\n"
            "ADDRESS: " + wallet.address();

        return result.c_str();

    } catch (const std::exception& e) {
        result = std::string("ERROR: ") + e.what();
        return result.c_str();
    }
}

extern "C" const char* larb_get_blockchain_info() {
    static std::string info;

    std::lock_guard<std::mutex> lock(active_mutex);

    if (!active_node) {
        info = "STATUS: STOPPED\\n";
        return info.c_str();
    }

    const auto& blockchain = active_node->blockchain();
    const std::size_t height = blockchain.size() - 1;

    info.clear();

    info += "STATUS: RUNNING\\n";
    info += "P2P PORT: 8333\\n";
    info += "BLOCK HEIGHT: ";
    info += std::to_string(height);
    info += "\\n\\n";

    info += "GENESIS HASH:\\n";
    info += blockchain.at(0).hash();
    info += "\\n\\n";

    info += "LATEST BLOCK HASH:\\n";
    info += blockchain.at(height).hash();
    info += "\\n\\n";

    info += "PEERS: ";

    if (active_server) {
        info += std::to_string(active_server->peer_count());
    } else {
        info += "0";
    }

    info += "\\n";

    return info.c_str();
}

int main(int argc, char* argv[]) {
    return run_node(argc, argv);
}
