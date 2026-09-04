#include "src/p2p.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>

int main() {
    std::cout << "=== LARB GET_CHAIN TEST ===\n";

    constexpr std::uint16_t port = 18446;

    larb::P2PServer server(port);

    if (!server.start()) {
        std::cerr << "Server start: FAIL\n";
        return 1;
    }

    std::cout << "Server start: OK\n";

    std::thread server_thread([&server]() {
        server.serve_once();
    });

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd < 0) {
        server.stop();
        server_thread.join();
        std::cerr << "Client socket: FAIL\n";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &address.sin_addr
    );

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
            0
        ) != static_cast<ssize_t>(request.size())) {
        close(client_fd);
        server.stop();
        server_thread.join();
        std::cerr << "GET_CHAIN send: FAIL\n";
        return 1;
    }

    std::cout << "GET_CHAIN send: OK\n";

    char buffer[1024]{};

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
        std::cerr << "Response receive: FAIL\n";
        return 1;
    }

    std::string response(
        buffer,
        static_cast<std::size_t>(received)
    );

    if (response != "CHAIN_REQUESTED") {
        std::cerr << "CHAIN_REQUESTED response: FAIL\n";
        return 1;
    }

    std::cout << "CHAIN_REQUESTED response: OK\n";

    std::cout << "=== ALL GET_CHAIN TESTS PASSED ===\n";

    return 0;
}
