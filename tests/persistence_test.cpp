#include "src/persistence.h"
#include "src/consensus/constants.h"
#include "src/genesis.h"
#include "src/consensus/pow.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB F4 PERSISTENCE TEST ===\n";

    const std::string path =
        "/tmp/larb_f4_state.dat";

    std::remove(path.c_str());

    larb::Block genesis =
        larb::Genesis::create();

    larb::Blockchain chain(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    larb::Block block1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {"persistence-tx-1"},
            1700000001,
            larb::INITIAL_POW_DIFFICULTY
        );

    if (!chain.add_block(block1)) {
        std::cerr << "Chain preparation: FAIL\n";
        return 1;
    }

    larb::UTXOSet utxos;

    const std::string txid =
        block1.hash();

    utxos.add(
        larb::UTXO{
            larb::OutPoint{
                txid,
                0
            },
            larb::TransactionOutput{
                123456,
                "larb1qexample"
            }
        }
    );

    if (!larb::Persistence::save(
            path,
            chain,
            utxos)) {
        std::cerr << "Save: FAIL\n";
        return 1;
    }

    std::cout << "Atomic save: OK\n";

    larb::Blockchain restored_chain(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    larb::UTXOSet restored_utxos;

    if (!larb::Persistence::load(
            path,
            restored_chain,
            restored_utxos)) {
        std::cerr << "Load: FAIL\n";
        return 1;
    }

    std::cout << "Load: OK\n";

    if (restored_chain.size() !=
        chain.size()) {
        std::cerr << "Chain state: FAIL\n";
        return 1;
    }

    if (restored_chain.at(
            restored_chain.size() - 1
        ).hash() !=
        chain.at(chain.size() - 1).hash()) {
        std::cerr << "Tip hash: FAIL\n";
        return 1;
    }

    std::cout << "Chain metadata: OK\n";

    if (restored_utxos.size() !=
        utxos.size()) {
        std::cerr << "UTXO state: FAIL\n";
        return 1;
    }

    const larb::OutPoint point{
        txid,
        0
    };

    const larb::UTXO* restored =
        restored_utxos.find(point);

    if (restored == nullptr ||
        restored->output.amount != 123456 ||
        restored->output.script_pubkey !=
            "larb1qexample") {
        std::cerr << "UTXO contents: FAIL\n";
        return 1;
    }

    std::cout << "UTXO state: OK\n";

    if (!restored_chain.is_valid()) {
        std::cerr << "Restart validation: FAIL\n";
        return 1;
    }

    std::cout << "Restart recovery: OK\n";

    {
        std::fstream corrupt(
            path,
            std::ios::in |
            std::ios::out |
            std::ios::binary
        );

        if (!corrupt) {
            std::cerr << "Corruption setup: FAIL\n";
            return 1;
        }

        char byte = 0;
        corrupt.read(&byte, 1);
        corrupt.seekp(0);
        byte ^= 0x01;
        corrupt.write(&byte, 1);
    }

    larb::Blockchain corrupt_chain(
        genesis,
        larb::INITIAL_POW_DIFFICULTY
    );

    larb::UTXOSet corrupt_utxos;

    if (larb::Persistence::load(
            path,
            corrupt_chain,
            corrupt_utxos)) {
        std::cerr << "Corrupt DB accepted: FAIL\n";
        return 1;
    }

    std::cout << "Corrupt database detection: OK\n";

    std::remove(path.c_str());

    std::cout << "=== ALL F4 PERSISTENCE TESTS PASSED ===\n";
    return 0;
}
