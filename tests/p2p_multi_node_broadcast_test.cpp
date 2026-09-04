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

std::string serialize_block(
    const larb::Block& block
) {
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
        out << tx.size()
            << ':'
            << tx
            << '\n';
    }

    return out.str();
}

std::string submit_message(
    const larb::Block& block
) {
    return "SUBMIT_BLOCK\n" +
           serialize_block(block);
}

bool broadcast_block(
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

bool check_same_tip(
    const larb::Node& a,
    const larb::Node& b
) {
    if (a.chain_size() != b.chain_size()) {
        return false;
    }

    const std::string tip_a =
        a.blockchain()
            .at(a.chain_size() - 1)
            .hash();

    const std::string tip_b =
        b.blockchain()
            .at(b.chain_size() - 1)
            .hash();

    return tip_a == tip_b;
}

}

int main() {
    std::cout
        << "=== LARB P2P MULTI NODE BROADCAST TEST ===\n";

    constexpr std::uint32_t difficulty = 3;

    constexpr std::uint16_t port_b = 18450;
    constexpr std::uint16_t port_c = 18451;

    /*
     * =========================
     * COMMON GENESIS
     * =========================
     */

    const larb::Block genesis =
        larb::Genesis::create();

    larb::Node node_a(
        genesis,
        difficulty
    );

    larb::Node node_b(
        genesis,
        difficulty
    );

    larb::Node node_c(
        genesis,
        difficulty
    );

    std::cout
        << "Node A prepared: OK\n";

    std::cout
        << "Node B prepared: OK\n";

    std::cout
        << "Node C prepared: OK\n";

    /*
     * =========================
     * NODE B SERVER
     * =========================
     */

    larb::P2PServer server_b(port_b);

    server_b.set_node(&node_b);

    if (!server_b.start()) {
        std::cerr
            << "Node B TCP server: FAIL\n";
        return 1;
    }

    /*
     * =========================
     * NODE C SERVER
     * =========================
     */

    larb::P2PServer server_c(port_c);

    server_c.set_node(&node_c);

    if (!server_c.start()) {
        server_b.stop();

        std::cerr
            << "Node C TCP server: FAIL\n";
        return 1;
    }

    std::cout
        << "Node B TCP server: OK\n";

    std::cout
        << "Node C TCP server: OK\n";

    /*
     * Each server handles one connection.
     */

    std::thread thread_b([&server_b]() {
        server_b.serve_once();
    });

    std::thread thread_c([&server_c]() {
        server_c.serve_once();
    });

    /*
     * =========================
     * MINE BLOCK ON NODE A
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
            1700000200,
            difficulty
        );

    if (!node_a.receive_block(block)) {
        server_b.stop();
        server_c.stop();

        thread_b.join();
        thread_c.join();

        std::cerr
            << "Node A block acceptance: FAIL\n";

        return 1;
    }

    std::cout
        << "Block mined by Node A: OK\n";

    /*
     * =========================
     * A -> B
     * =========================
     */

    std::string response_b;

    if (!broadcast_block(
            block,
            port_b,
            response_b)) {

        server_b.stop();
        server_c.stop();

        thread_b.join();
        thread_c.join();

        std::cerr
            << "Broadcast A -> B: FAIL\n";

        return 1;
    }

    std::cout
        << "Broadcast A -> B: OK\n";

    /*
     * =========================
     * A -> C
     * =========================
     */

    std::string response_c;

    if (!broadcast_block(
            block,
            port_c,
            response_c)) {

        server_b.stop();
        server_c.stop();

        thread_b.join();
        thread_c.join();

        std::cerr
            << "Broadcast A -> C: FAIL\n";

        return 1;
    }

    std::cout
        << "Broadcast A -> C: OK\n";

    /*
     * =========================
     * WAIT SERVERS
     * =========================
     */

    thread_b.join();
    thread_c.join();

    server_b.stop();
    server_c.stop();

    /*
     * =========================
     * RESPONSES
     * =========================
     */

    if (response_b != "BLOCK_ACCEPTED") {
        std::cerr
            << "Node B response: ["
            << response_b
            << "]\n";

        return 1;
    }

    if (response_c != "BLOCK_ACCEPTED") {
        std::cerr
            << "Node C response: ["
            << response_c
            << "]\n";

        return 1;
    }

    std::cout
        << "Node B accepted block: OK\n";

    std::cout
        << "Node C accepted block: OK\n";

    /*
     * =========================
     * CONSENSUS
     * =========================
     */

    if (!check_same_tip(node_a, node_b)) {
        std::cerr
            << "A/B tip equality: FAIL\n";
        return 1;
    }

    std::cout
        << "A/B tip equality: OK\n";

    if (!check_same_tip(node_a, node_c)) {
        std::cerr
            << "A/C tip equality: FAIL\n";
        return 1;
    }

    std::cout
        << "A/C tip equality: OK\n";

    /*
     * =========================
     * VALIDATION
     * =========================
     */

    if (!node_a.is_chain_valid()) {
        std::cerr
            << "Node A validation: FAIL\n";
        return 1;
    }

    if (!node_b.is_chain_valid()) {
        std::cerr
            << "Node B validation: FAIL\n";
        return 1;
    }

    if (!node_c.is_chain_valid()) {
        std::cerr
            << "Node C validation: FAIL\n";
        return 1;
    }

    std::cout
        << "All node chain validation: OK\n";

    /*
     * =========================
     * UTXO
     * =========================
     */

    if (node_a.utxos().size() !=
        node_b.utxos().size()) {

        std::cerr
            << "A/B UTXO equality: FAIL\n";
        return 1;
    }

    if (node_a.utxos().size() !=
        node_c.utxos().size()) {

        std::cerr
            << "A/C UTXO equality: FAIL\n";
        return 1;
    }

    std::cout
        << "All node UTXO equality: OK\n";

    /*
     * =========================
     * RESULT
     * =========================
     */

    std::cout
        << "Node A chain size: "
        << node_a.chain_size()
        << '\n';

    std::cout
        << "Node B chain size: "
        << node_b.chain_size()
        << '\n';

    std::cout
        << "Node C chain size: "
        << node_c.chain_size()
        << '\n';

    std::cout
        << "=== ALL P2P MULTI NODE BROADCAST TESTS PASSED ===\n";

    return 0;
}
