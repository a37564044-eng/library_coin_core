#include "src/genesis.h"
#include "src/blockchain.h"
#include "src/persistence.h"
#include "src/consensus/constants.h"
#include "src/utxo_set.h"

#include <iostream>

int main() {
    larb::Blockchain blockchain(
        larb::Genesis::create(),
        larb::INITIAL_POW_DIFFICULTY
    );

    larb::UTXOSet utxos;

    if (!larb::Persistence::load(
            "larb_chain.dat",
            blockchain,
            utxos)) {
        std::cerr << "LOAD FAILED\n";
        return 1;
    }

    std::cout << "Chain height: "
              << blockchain.size() - 1 << "\n";

    std::cout << "UTXO count: "
              << utxos.size() << "\n";

    for (const auto& u : utxos.snapshot()) {
        std::cout << "TXID: "
                  << u.outpoint.txid << "\n";
        std::cout << "INDEX: "
                  << u.outpoint.output_index << "\n";
        std::cout << "AMOUNT: "
                  << u.output.amount << "\n";
        std::cout << "SCRIPT: "
                  << u.output.script_pubkey << "\n";
        std::cout << "-----\n";
    }

    return 0;
}
