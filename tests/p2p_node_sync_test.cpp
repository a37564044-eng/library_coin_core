#include "src/genesis.h"
#include "src/node.h"
#include "src/p2p.h"
#include "src/chain_codec.h"
#include "src/transaction.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>

namespace {

bool send_all(
    int fd,
    const std::string& data
) {
    std::size_t sent = 0;

    while (sent < data.size()) {
        const ssize_t n = send(
            fd,
            data.data() + sent,
            data.size() - sent,
            0
        );

        if (n <= 0) {
            return false;
        }

        sent += static_cast<std::size_t>(n);
    }

    return true;
}

std::string recv_all(int fd) {
    std::string result;
    char buffer[65536];

    for (;;) {
        const ssize_t n = recv(
            fd,
            buffer,
            sizeof(buffer),
            0
        );

        if (n <= 0) {
            break;
        }

        result.append(
            buffer,
            static_cast<std::size_t>(n)
        );

        /*
         * Prototype payload kita masih kecil.
         * Stop setelah koneksi ditutup oleh server.
         */
    }

    return result;
}

}

int main() {
    std::cout
        << "=== LARB P2P NODE SYNC TEST ===\n";

    constexpr std::uint32_t difficulty = 3;
    constexpr std::uint16_t port = 18448;

    /*
     * Genesis yang sama untuk kedua node.
     */
    larb::Block genesis =
        larb::Genesis::create();

    larb::Node node_a(
        genesis,
        difficulty
    );

    larb::Node node_b(
        genesis,
        difficulty
    );

    /*
     * =========================
     * NODE A: Genesis -> B1 -> B2
     * =========================
     */

    larb::Transaction coinbase1 =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-NODE-A",
            1
        );

    larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase1.serialize()},
            1700000001,
            difficulty
        );

    if (!node_a.receive_block(block1)) {
        std::cerr
            << "Node A block 1: FAIL\n";
        return 1;
    }

    larb::Transaction coinbase2 =
        larb::Transaction::coinbase(
            larb::get_block_reward(2),
            "LARB-NODE-A",
            2
        );

    larb::Block block2 =
        larb::mine_block(
            1,
            block1.hash(),
            {coinbase2.serialize()},
            1700000002,
            difficulty
        );

    if (!node_a.receive_block(block2)) {
        std::cerr
            << "Node A block 2: FAIL\n";
        return 1;
    }

    /*
     * =========================
     * NODE B hanya sampai B1
     * =========================
     */

    if (!node_b.receive_block(block1)) {
        std::cerr
            << "Node B block 1: FAIL\n";
        return 1;
    }

    std::cout
        << "Node A chain prepared: OK\n";

    std::cout
        << "Node B shorter chain prepared: OK\n";

    /*
     * =========================
     * TCP SERVER NODE A
     * =========================
     */

    larb::P2PServer server(port);

    server.set_blockchain(
        &node_a.blockchain()
    );

    if (!server.start()) {
        std::cerr
            << "Node A TCP server: FAIL\n";
        return 1;
    }

    std::cout
        << "Node A TCP server: OK\n";

    std::thread server_thread([&server]() {
        server.serve_once();
    });

    /*
     * =========================
     * NODE B CONNECT
     * =========================
     */

    int client_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (client_fd < 0) {
        server.stop();
        server_thread.join();

        std::cerr
            << "Node B socket: FAIL\n";
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

        std::cerr
            << "Address: FAIL\n";
        return 1;
    }

    if (connect(
            client_fd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)) < 0) {

        close(client_fd);
        server.stop();
        server_thread.join();

        std::cerr
            << "Node B TCP connect: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B TCP connect: OK\n";

    /*
     * Request chain dari Node A.
     */
    const std::string request =
        "GET_CHAIN";

    if (!send_all(
            client_fd,
            request)) {

        close(client_fd);
        server.stop();
        server_thread.join();

        std::cerr
            << "GET_CHAIN send: FAIL\n";
        return 1;
    }

    std::cout
        << "GET_CHAIN send: OK\n";

    /*
     * Server menutup koneksi setelah response.
     */
    const std::string response =
        recv_all(client_fd);

    close(client_fd);

    server_thread.join();
    server.stop();

    if (response.empty()) {
        std::cerr
            << "Chain response: FAIL\n";
        return 1;
    }

    std::cout
        << "Chain response received: OK\n";

    std::cout
        << "Received bytes: "
        << response.size()
        << '\n';

    /*
     * =========================
     * DESERIALIZE
     * =========================
     */

    const auto decoded =
        larb::deserialize_chain(response);

    if (!decoded.has_value()) {
        std::cerr
            << "Deserialize chain: FAIL\n";
        return 1;
    }

    std::cout
        << "Deserialize chain: OK\n";

    /*
     * Pastikan chain yang diterima memang
     * chain Node A.
     */
    if (decoded->size() !=
        node_a.chain_size()) {

        std::cerr
            << "Received chain size: FAIL\n";
        return 1;
    }

    std::cout
        << "Received chain size: OK\n";

    /*
     * =========================
     * ADOPT DI NODE B
     * =========================
     */

    if (!node_b.adopt_chain(
            *decoded)) {

        std::cerr
            << "Node B adopt chain: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B adopt chain: OK\n";

    /*
     * =========================
     * FINAL CONSENSUS CHECK
     * =========================
     */

    if (node_a.chain_size() !=
        node_b.chain_size()) {

        std::cerr
            << "Chain size equality: FAIL\n";
        return 1;
    }

    std::cout
        << "Chain size equality: OK\n";

    const std::string tip_a =
        node_a.blockchain()
            .at(node_a.chain_size() - 1)
            .hash();

    const std::string tip_b =
        node_b.blockchain()
            .at(node_b.chain_size() - 1)
            .hash();

    if (tip_a != tip_b) {
        std::cerr
            << "Tip hash equality: FAIL\n";
        return 1;
    }

    std::cout
        << "Tip hash equality: OK\n";

    if (!node_a.is_chain_valid()) {
        std::cerr
            << "Node A chain validation: FAIL\n";
        return 1;
    }

    std::cout
        << "Node A chain validation: OK\n";

    if (!node_b.is_chain_valid()) {
        std::cerr
            << "Node B chain validation: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B chain validation: OK\n";

    if (node_a.utxos().size() !=
        node_b.utxos().size()) {

        std::cerr
            << "UTXO equality: FAIL\n";
        return 1;
    }

    std::cout
        << "UTXO equality: OK\n";

    std::cout
        << "Node A chain size: "
        << node_a.chain_size()
        << '\n';

    std::cout
        << "Node B chain size: "
        << node_b.chain_size()
        << '\n';

    std::cout
        << "=== ALL P2P NODE SYNC TESTS PASSED ===\n";

    return 0;
}
