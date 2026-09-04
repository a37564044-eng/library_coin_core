#include "src/p2p.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <thread>

int main() {
    std::cout << "=== LARB TCP P2P TEST ===\n";

    constexpr std::uint16_t port = 18445;

    larb::P2PServer server(port);

    if (!server.start()) {
        std::cerr << "TCP server start: FAIL\n";
        return 1;
    }

    std::cout << "TCP server start: OK\n";

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
        std::cerr << "TCP client socket: FAIL\n";
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &server_address.sin_addr
    );

    if (connect(
            client_fd,
            reinterpret_cast<sockaddr*>(&server_address),
            sizeof(server_address)) < 0) {
        close(client_fd);
        server.stop();
        server_thread.join();
        std::cerr << "TCP connect: FAIL\n";
        return 1;
    }

    std::cout << "TCP connect: OK\n";

    const std::string ping = "PING";

    if (send(
            client_fd,
            ping.data(),
            ping.size(),
            0
        ) != static_cast<ssize_t>(ping.size())) {
        close(client_fd);
        server.stop();
        server_thread.join();
        std::cerr << "PING send: FAIL\n";
        return 1;
    }

    std::cout << "PING send: OK\n";

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
        std::cerr << "PONG receive: FAIL\n";
        return 1;
    }

    std::string response(
        buffer,
        static_cast<std::size_t>(received)
    );

    if (response != "PONG") {
        std::cerr << "PONG response: FAIL\n";
        return 1;
    }

    std::cout << "PONG receive: OK\n";

    std::cout << "=== ALL TCP P2P TESTS PASSED ===\n";

    return 0;
}
