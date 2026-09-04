#include "p2p.h"
#include "chain_codec.h"
#include "block.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace larb {

namespace {

std::string serialize_single_block(const Block& block) {
    const auto& h = block.header();
    std::ostringstream out;

    out << h.version << '\n';
    out << h.previous_hash << '\n';
    out << h.merkle_root << '\n';
    out << h.timestamp << '\n';
    out << h.nonce << '\n';

    const auto& transactions = block.transactions();
    out << transactions.size() << '\n';

    for (const auto& tx : transactions) {
        out << tx.size() << ':' << tx << '\n';
    }

    return out.str();
}

Block deserialize_single_block(const std::string& data) {
    std::size_t pos = 0;

    auto read_line = [&]() -> std::string {
        const std::size_t end = data.find('\n', pos);

        if (end == std::string::npos) {
            throw std::runtime_error("missing newline");
        }

        std::string result = data.substr(pos, end - pos);
        pos = end + 1;
        return result;
    };

    const std::uint32_t version =
        static_cast<std::uint32_t>(std::stoul(read_line()));

    const std::string previous_hash = read_line();
    const std::string merkle_root = read_line();

    const std::uint64_t timestamp =
        std::stoull(read_line());

    const std::uint64_t nonce =
        std::stoull(read_line());

    const std::size_t tx_count =
        static_cast<std::size_t>(std::stoull(read_line()));

    std::vector<std::string> transactions;
    transactions.reserve(tx_count);

    for (std::size_t i = 0; i < tx_count; ++i) {
        const std::size_t colon = data.find(':', pos);

        if (colon == std::string::npos) {
            throw std::runtime_error("invalid transaction");
        }

        const std::size_t tx_size =
            std::stoull(data.substr(pos, colon - pos));

        pos = colon + 1;

        if (tx_size > data.size() - pos) {
            throw std::runtime_error("transaction truncated");
        }

        std::string tx = data.substr(pos, tx_size);
        pos += tx_size;

        if (pos >= data.size() || data[pos] != '\n') {
            throw std::runtime_error(
                "missing transaction delimiter"
            );
        }

        ++pos;
        transactions.push_back(std::move(tx));
    }

    if (pos != data.size()) {
        throw std::runtime_error("unexpected trailing data");
    }

    Block block(
        version,
        previous_hash,
        transactions,
        timestamp,
        nonce
    );

    if (block.header().merkle_root != merkle_root) {
        throw std::runtime_error("merkle root mismatch");
    }

    return block;
}

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

bool send_block_to_peer(
    const Block& block,
    const std::string& host,
    std::uint16_t port
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
            host.c_str(),
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
        "SUBMIT_BLOCK\n" + serialize_single_block(block);

    if (!send_all(fd, message)) {
        close(fd);
        return false;
    }

    char buffer[4096]{};

    const ssize_t received = recv(
        fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (received <= 0) {
        close(fd);
        return false;
    }

    const std::string response(
        buffer,
        static_cast<std::size_t>(received)
    );

    close(fd);

    return response == "BLOCK_ACCEPTED";
}

} // anonymous namespace


P2PServer::P2PServer(std::uint16_t port)
    : port_(port),
      server_fd_(-1),
      blockchain_(nullptr),
      node_(nullptr),
      peers_{} {
}


bool P2PServer::start() {
    server_fd_ = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_fd_ < 0) {
        return false;
    }

    int opt = 1;

    if (setsockopt(
            server_fd_,
            SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)) < 0) {
        stop();
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port_);

    if (bind(
            server_fd_,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)) < 0) {
        stop();
        return false;
    }

    if (listen(server_fd_, 8) < 0) {
        stop();
        return false;
    }

    return true;
}


void P2PServer::stop() {
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
}


void P2PServer::set_blockchain(
    const Blockchain* blockchain
) {
    blockchain_ = blockchain;
}


void P2PServer::set_node(Node* node) {
    node_ = node;
}


void P2PServer::add_peer(
    const std::string& host,
    std::uint16_t port
) {
    if (host.empty() || port == 0) {
        return;
    }

    if (host == "127.0.0.1" && port == port_) {
        return;
    }

    for (const auto& peer : peers_) {
        if (peer.host == host && peer.port == port) {
            return;
        }
    }

    peers_.push_back(Peer{host, port});
}


void P2PServer::clear_peers() {
    peers_.clear();
}


std::size_t P2PServer::peer_count() const {
    return peers_.size();
}


bool P2PServer::sync_from_peer(
    const std::string& host,
    std::uint16_t port
) {
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        close(fd);
        return false;
    }

    if (connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(fd);
        return false;
    }

    const std::string request = "GET_CHAIN";
    if (!send_all(fd, request)) {
        close(fd);
        return false;
    }

    std::string data;
    char buffer[65536];
    ssize_t received;

    while ((received = recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        data.append(buffer, static_cast<std::size_t>(received));
    }

    close(fd);

    if (data.empty() || node_ == nullptr) return false;

    const auto candidate = deserialize_chain(data);
    if (!candidate.has_value()) return false;

    return node_->adopt_chain(*candidate);
}

bool P2PServer::broadcast_block(
    const Block& block
) const {
    if (peers_.empty()) {
        return true;
    }

    bool all_ok = true;

    for (const auto& peer : peers_) {
        if (!send_block_to_peer(
                block,
                peer.host,
                peer.port)) {
            all_ok = false;
        }
    }

    return all_ok;
}


bool P2PServer::handle_message(
    const std::string& message,
    std::string& response
) const {
    if (message == "PING") {
        response = "PONG";
        return true;
    }

    if (message == "GET_CHAIN") {
        if (blockchain_ == nullptr) {
            response = "CHAIN_REQUESTED";
            return true;
        }

        response = serialize_chain(*blockchain_);
        return true;
    }

    const std::string prefix = "SUBMIT_BLOCK\n";

    if (message.rfind(prefix, 0) == 0) {
        if (node_ == nullptr) {
            response = "BLOCK_REJECTED_PARSE";
            return false;
        }

        try {
            const std::string payload =
                message.substr(prefix.size());

            Block block =
                deserialize_single_block(payload);

            if (!node_->receive_block(block)) {
                response = "BLOCK_REJECTED";
                return false;
            }

            response = "BLOCK_ACCEPTED";
            return true;

        } catch (...) {
            response = "BLOCK_REJECTED";
            return false;
        }
    }

    response = "ERROR";
    return false;
}


bool P2PServer::serve_once() {
    if (server_fd_ < 0) {
        return false;
    }

    sockaddr_in client_address{};
    socklen_t client_length =
        sizeof(client_address);

    const int client_fd = accept(
        server_fd_,
        reinterpret_cast<sockaddr*>(&client_address),
        &client_length
    );

    if (client_fd < 0) {
        return false;
    }

    char buffer[65536]{};

    const ssize_t received = recv(
        client_fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (received <= 0) {
        close(client_fd);
        return false;
    }

    const std::string message(
        buffer,
        static_cast<std::size_t>(received)
    );

    std::string response;

    handle_message(message, response);
    send_all(client_fd, response);

    close(client_fd);
    return true;
}

} // namespace larb
