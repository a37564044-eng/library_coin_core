#include "src/genesis.h"
#include "src/node.h"
#include "src/p2p.h"
#include "src/consensus/pow.h"
#include "src/transaction.h"
#include "src/consensus/constants.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

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

}

int main() {
    std::cout << "=== LARB P2P SUBMIT BLOCK TEST ===\n";

    larb::Block genesis =
        larb::Genesis::create();

    larb::Node node(genesis, 3);

    larb::P2PServer server(18446);

    server.set_node(&node);

    std::string response;

    /*
     * 1. Valid mined block.
     */
    const larb::Transaction coinbase = larb::Transaction::coinbase(
        larb::get_block_reward(1),
        "LARB-MINER",
        1
    );

    const std::string coinbase_tx = coinbase.serialize();

    larb::Block valid_block =
        larb::mine_block(
            1,
            genesis.hash(),
            {coinbase_tx},
            2,
            3
        );

    if (!server.handle_message(
            submit_message(valid_block),
            response)) {

        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Valid block rejection: FAIL\n";
        return 1;
    }

    if (response != "BLOCK_ACCEPTED") {
        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Valid block response: FAIL\n";
        return 1;
    }

    std::cout
        << "Valid mined block: OK\n";

    /*
     * 2. Wrong previous hash.
     */
    larb::Block wrong_previous =
        larb::mine_block(
            1,
            "WRONG_PREVIOUS_HASH",
            {"TX2"},
            3,
            3
        );

    response.clear();

    if (server.handle_message(
            submit_message(wrong_previous),
            response)) {

        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Wrong previous hash rejection: FAIL\n";
        return 1;
    }

    if (response != "BLOCK_REJECTED") {
        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Wrong previous hash response: FAIL\n";
        return 1;
    }

    std::cout
        << "Reject wrong previous hash: OK\n";

    /*
     * 3. Invalid PoW.
     */
    larb::Block invalid_pow(
        1,
        valid_block.hash(),
        {"TX3"},
        4,
        0
    );

    response.clear();

    if (server.handle_message(
            submit_message(invalid_pow),
            response)) {

        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Invalid PoW rejection: FAIL\n";
        return 1;
    }

    if (response != "BLOCK_REJECTED") {
        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Invalid PoW response: FAIL\n";
        return 1;
    }

    std::cout
        << "Reject invalid PoW: OK\n";

    /*
     * 4. Merkle root tampering.
     */
    larb::Block bad_merkle(
        1,
        valid_block.hash(),
        {"TX4"},
        5,
        0
    );

    response.clear();

    std::string tampered =
        serialize_block(bad_merkle);

    /*
     * Ganti Merkle root menjadi nilai palsu.
     */
    std::size_t first = tampered.find('\n');
    std::size_t second =
        tampered.find('\n', first + 1);

    std::size_t third =
        tampered.find('\n', second + 1);

    if (second != std::string::npos &&
        third != std::string::npos) {

        tampered.replace(
            second + 1,
            third - second - 1,
            "BAD_MERKLE_ROOT"
        );
    }

    if (server.handle_message(
            "SUBMIT_BLOCK\n" + tampered,
            response)) {

        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Bad Merkle rejection: FAIL\n";
        return 1;
    }

    if (response != "BLOCK_REJECTED") {
        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Bad Merkle response: FAIL\n";
        return 1;
    }

    std::cout
        << "Reject bad Merkle root: OK\n";

    /*
     * Pastikan hanya block valid yang masuk.
     */
    if (node.chain_size() != 2) {
        std::cerr << "RESPONSE=[" << response << "]\n"; std::cerr
            << "Final chain size: FAIL\n";
        return 1;
    }

    std::cout
        << "Final chain size: OK\n";

    std::cout
        << "=== ALL P2P SUBMIT BLOCK TESTS PASSED ===\n";

    return 0;
}
