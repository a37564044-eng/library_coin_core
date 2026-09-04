#pragma once

#include "blockchain.h"
#include "node.h"

#include <cstdint>
#include <string>
#include <vector>

namespace larb {

class P2PServer {
public:
    explicit P2PServer(std::uint16_t port);

    bool start();
    void stop();

    bool handle_message(
        const std::string& message,
        std::string& response
    ) const;

    bool serve_once();

    void set_blockchain(const Blockchain* blockchain);
    void set_node(Node* node);

    void add_peer(
        const std::string& host,
        std::uint16_t port
    );

    void clear_peers();

    std::size_t peer_count() const;

    bool sync_from_peer(const std::string& host, std::uint16_t port);
    bool broadcast_block(
        const Block& block
    ) const;

private:
    struct Peer {
        std::string host;
        std::uint16_t port;
    };

    std::uint16_t port_;
    int server_fd_;
    const Blockchain* blockchain_;
    Node* node_;
    std::vector<Peer> peers_;
};

} // namespace larb
