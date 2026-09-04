#include "src/block.h"
#include "src/blockchain.h"
#include "src/consensus/pow.h"
#include "src/p2p.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

int main() {
    std::cout << "=== LARB P2P CHAIN TEST ===\n";

    constexpr std::uint16_t port = 18447;
    constexpr std::uint32_t difficulty = 3;

    larb::Block genesis(
        1,
        "0",
        {"LARB Genesis Block"},
        1700000000,
        0
    );

    larb::Blockchain chain(genesis, difficulty);

    larb::Block block1 = larb::mine_block(
        1,
        genesis.hash(),
        {"transaction-1"},
        1700000001,
        difficulty
    );

    if (!chain.add_block(block1)) {
        std::cerr << "Add block: FAIL\n";
        return 1;
    }

    std::cout << "Add mined block: OK\n";
    std::cout << "Chain preparation: OK\n";

    larb::P2PServer server(port);
    server.set_blockchain(&chain);

    if (!server.start()) {
        std::cerr << "Server start: FAIL\n";
        return 1;
    }

    std::cout << "Server start: OK\n";

    std::thread server_thread([&server]() {
        server.serve_once();
    });

    int client_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (client_fd < 0) {
        server.stop();
        server_thread.join();
        std::cerr << "Client socket: FAIL\n";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(
            AF_INET,
            "127.0.0.1",
            &address.sin_addr) != 1) {
        close(client_fd);
        server.stop();
        server_thread.join();
        std::cerr << "Address: FAIL\n";
        return 1;
    }

    if (connect(
            client_fd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)) < 0) {
        close(client_fd);
        server.stop();
        server_thread.join();
        std::cerr << "TCP connect: FAIL\n";
        return 1;
    }

    std::cout << "TCP connect: OK\n";

    const std::string request = "GET_CHAIN";

    if (send(
            client_fd,
            request.data(),
            request.size(),
            0) != static_cast<ssize_t>(request.size())) {
        close(client_fd);
        server.stop();
        server_thread.join();
        std::cerr << "GET_CHAIN send: FAIL\n";
        return 1;
    }

    std::cout << "GET_CHAIN send: OK\n";

    char buffer[4096]{};

    ssize_t received = recv(
        client_fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    close(client_fd);
    server_thread.join();
    server.stop();

    if (received <= 0) {
        std::cerr << "CHAIN receive: FAIL\n";
        return 1;
    }

    std::string response(
        buffer,
        static_cast<std::size_t>(received)
    );

    if (response.find("LARB Genesis Block") ==
        std::string::npos) {
        std::cerr << "Genesis payload: FAIL\n";
        return 1;
    }

    if (response.find("transaction-1") ==
        std::string::npos) {
        std::cerr << "Transaction payload: FAIL\n";
        return 1;
    }

    std::cout << "CHAIN receive: OK\n";
    std::cout << "Genesis payload: OK\n";
    std::cout << "Transaction payload: OK\n";
    std::cout << "Received bytes: "
              << response.size() << '\n';

    std::cout << "=== ALL P2P CHAIN TESTS PASSED ===\n";

    return 0;
}
