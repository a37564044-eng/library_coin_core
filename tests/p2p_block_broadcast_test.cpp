#include "src/genesis.h"
#include "src/node.h"
#include "src/p2p.h"
#include "src/transaction.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace {

bool send_all(int fd, const std::string& data) {
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

std::string serialize_block(const larb::Block& block) {
    const auto& h = block.header();

    std::ostringstream out;

    out << h.version << '\n';
    out << h.previous_hash << '\n';
    out << h.merkle_root << '\n';
    out << h.timestamp << '\n';
    out << h.nonce << '\n';

    const auto& txs = block.transactions();

    out << txs.size() << '\n';

    for (const auto& tx : txs) {
        out << tx.size() << ':' << tx << '\n';
    }

    return out.str();
}

std::string submit_message(const larb::Block& block) {
    return "SUBMIT_BLOCK\n" + serialize_block(block);
}

bool send_block(
    const larb::Block& block,
    std::uint16_t port,
    std::string& response
) {
    const int fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (fd < 0) {
        return false;
    }

    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(
            AF_INET,
            "127.0.0.1",
            &address.sin_addr) != 1) {

        close(fd);
        return false;
    }

    if (connect(
            fd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)) < 0) {

        close(fd);
        return false;
    }

    const std::string message =
        submit_message(block);

    if (!send_all(fd, message)) {
        close(fd);
        return false;
    }

    char buffer[4096]{};

    const ssize_t received =
        recv(
            fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

    if (received <= 0) {
        close(fd);
        return false;
    }

    response.assign(
        buffer,
        static_cast<std::size_t>(received)
    );

    close(fd);

    return true;
}

}

int main() {
    std::cout
        << "=== LARB P2P BLOCK BROADCAST TEST ===\n";

    constexpr std::uint32_t difficulty = 3;
    constexpr std::uint16_t port = 18449;

    /*
     * =========================
     * COMMON GENESIS
     * =========================
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

    std::cout
        << "Node A prepared: OK\n";

    std::cout
        << "Node B prepared: OK\n";

    /*
     * =========================
     * NODE B SERVER
     * =========================
     */

    larb::P2PServer server(port);

    server.set_node(&node_b);

    if (!server.start()) {
        std::cerr
            << "Node B TCP server: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B TCP server: OK\n";

    std::thread server_thread([&server]() {
        server.serve_once();
    });

    /*
     * =========================
     * MINE BLOCK DI NODE A
     * =========================
     */

    const larb::Transaction coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            "LARB-NODE-A",
            1
        );

    const larb::Block block =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase.serialize()},
            1700000100,
            difficulty
        );

    /*
     * Node A menerima block hasil mining.
     */

    if (!node_a.receive_block(block)) {
        server.stop();
        server_thread.join();

        std::cerr
            << "Node A block acceptance: FAIL\n";
        return 1;
    }

    std::cout
        << "Block mined by Node A: OK\n";

    /*
     * =========================
     * BROADCAST A -> B
     * =========================
     */

    std::string response;

    if (!send_block(
            block,
            port,
            response)) {

        server.stop();
        server_thread.join();

        std::cerr
            << "Block broadcast: FAIL\n";
        return 1;
    }

    std::cout
        << "Block broadcast: OK\n";

    server_thread.join();
    server.stop();

    /*
     * =========================
     * NODE B RESULT
     * =========================
     */

    if (response != "BLOCK_ACCEPTED") {
        std::cerr
            << "RESPONSE=[" << response << "]\n"
            << "Node B accepted block: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B received block: OK\n";

    std::cout
        << "Node B accepted block: OK\n";

    /*
     * =========================
     * CONSENSUS CHECK
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

    if (!node_b.is_chain_valid()) {
        std::cerr
            << "Node B chain validation: FAIL\n";
        return 1;
    }

    std::cout
        << "Chain validation: OK\n";

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
        << "=== ALL P2P BLOCK BROADCAST TESTS PASSED ===\n";

    return 0;
}
