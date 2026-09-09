#include "src/blockchain.h"
#include "src/genesis.h"
#include "src/persistence.h"
#include "src/utxo_set.h"
#include <iostream>
#include <string>

int main() {
    larb::Blockchain chain(larb::Genesis::create(), 3);
    larb::UTXOSet utxos;

    if (!larb::Persistence::load("larb_chain.dat", chain, utxos)) {
        std::cerr << "Gagal memuat larb_chain.dat\n";
        return 1;
    }

    std::cout << "========== LARB BLOCKCHAIN ==========\n";
    std::cout << "Chain height: " << chain.size() << "\n\n";

    for (std::size_t i = 0; i < chain.size(); ++i) {
        const auto& block = chain.at(i);
        const auto& h = block.header();

        std::cout << "========== BLOCK #" << i << " ==========\n";
        std::cout << "Version      : " << h.version << "\n";
        std::cout << "Previous hash: " << h.previous_hash << "\n";
        std::cout << "Merkle root  : " << h.merkle_root << "\n";
        std::cout << "Timestamp    : " << h.timestamp << "\n";
        std::cout << "Nonce        : " << h.nonce << "\n";
        std::cout << "Hash         : " << block.hash() << "\n";

        const auto& txs = block.transactions();
        std::cout << "Transactions : " << txs.size() << "\n";

        for (std::size_t j = 0; j < txs.size(); ++j) {
            std::cout << "\n--- Transaction #" << j << " ---\n";
            std::cout << txs[j] << "\n";
        }

        std::cout << "\n";
    }

    std::cout << "=====================================\n";
    return 0;
}
