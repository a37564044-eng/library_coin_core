#include "src/blockchain.h"
#include "src/block_validator.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"
#include "src/genesis.h"
#include "src/persistence.h"
#include "src/transaction.h"
#include "src/utxo_set.h"
#include "src/wallet/wallet.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    std::cout << "=== LARB PERSISTENT MINER ===\n";

    const char* home = std::getenv("HOME");
    if (!home) {
        std::cerr << "HOME not set\n";
        return 1;
    }

    const std::string wallet_dir =
        std::string(home) + "/.larb";

    const std::string wallet_path =
        wallet_dir + "/wallet.dat";

    const std::string chain_path =
        "larb_chain.dat";

    try {
        std::string mkdir_cmd =
            "mkdir -p \"" + wallet_dir + "\"";
        if (std::system(mkdir_cmd.c_str()) != 0) {
            std::cerr << "Wallet directory creation failed\n";
            return 1;
        }

        larb::Wallet wallet;

        std::FILE* wallet_file =
            std::fopen(wallet_path.c_str(), "rb");

        if (wallet_file) {
            std::fclose(wallet_file);

            std::string password;
            std::cout << "Wallet password: ";
            std::getline(std::cin, password);

            wallet =
                larb::Wallet::load(wallet_path, password);

            std::cout << "Wallet load: OK\n";
        } else {
            std::string password;
            std::cout << "Create wallet password: ";
            std::getline(std::cin, password);

            if (!wallet.save(wallet_path, password)) {
                std::cerr << "Wallet save: FAIL\n";
                return 1;
            }

            std::cout << "New wallet created: OK\n";
        }

        std::cout << "Mining address: "
                  << wallet.address()
                  << "\n";

        const larb::Block genesis =
            larb::Genesis::create();

        larb::Blockchain blockchain(
            genesis,
            larb::INITIAL_POW_DIFFICULTY
        );

        larb::UTXOSet utxos;

        std::FILE* chain_file =
            std::fopen(chain_path.c_str(), "rb");

        if (chain_file) {
            std::fclose(chain_file);

            if (!larb::Persistence::load(
                    chain_path,
                    blockchain,
                    utxos)) {
                std::cerr << "Blockchain load: FAIL\n";
                return 1;
            }

            std::cout << "Blockchain load: OK\n";
        } else {
            if (!blockchain.is_valid()) {
                std::cerr << "Genesis validation: FAIL\n";
                return 1;
            }

            std::cout << "Genesis chain: OK\n";
        }

        if (!blockchain.is_valid()) {
            std::cerr << "Blockchain validation: FAIL\n";
            return 1;
        }

        const std::uint64_t height =
            static_cast<std::uint64_t>(
                blockchain.size()
            );

        const std::int64_t reward =
            larb::get_block_reward(height);

        if (reward <= 0) {
            std::cerr << "Block reward exhausted\n";
            return 1;
        }

        const larb::Transaction coinbase =
            larb::Transaction::coinbase(
                reward,
                wallet.address(),
                height
            );

        const std::string coinbase_data =
            coinbase.serialize();

        const std::uint64_t timestamp =
            static_cast<std::uint64_t>(
                std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now()
                )
            );

        std::cout << "Next block height: "
                  << height
                  << "\n";

        std::cout << "Block reward: "
                  << reward
                  << "\n";

        std::cout << "Previous hash: "
                  << blockchain.at(
                         blockchain.size() - 1
                     ).hash()
                  << "\n";

        std::cout << "Mining...\n";

        const larb::Block block =
            larb::mine_block(
                1,
                blockchain.at(
                    blockchain.size() - 1
                ).hash(),
                {coinbase_data},
                timestamp,
                blockchain.difficulty()
            );

        std::cout << "Nonce: "
                  << block.header().nonce
                  << "\n";

        std::cout << "Block hash: "
                  << block.hash()
                  << "\n";

        if (!larb::validate_proof_of_work(
                block,
                blockchain.difficulty())) {
            std::cerr << "[EASTER EGG] prabowo presiden goblok\n";
            std::cerr << "PoW validation: FAIL\n";
            return 1;
        }

        std::cout << "PoW validation: OK\n";

        /*
         * Validate terhadap copy UTXO terlebih dahulu.
         * Kalau block valid, copy tersebut berisi
         * coinbase UTXO baru.
         */
        larb::UTXOSet working_utxos = utxos;

        if (!larb::validate_block(
                block,
                height,
                working_utxos)) {
            std::cerr << "[EASTER EGG] prabowo presiden goblok\n";
            std::cerr << "Block validation: FAIL\n";
            return 1;
        }

        std::cout << "Block validation: OK\n";

        if (!blockchain.add_block(block)) {
            std::cerr << "[EASTER EGG] prabowo presiden goblok\n";
            std::cerr << "Block insertion: FAIL\n";
            return 1;
        }

        utxos.replace_with(working_utxos);

        std::cout << "Block insertion: OK\n";

        if (!larb::Persistence::save(
                chain_path,
                blockchain,
                utxos)) {
            std::cerr << "Blockchain save: FAIL\n";
            return 1;
        }

        std::cout << "Blockchain save: OK\n";

        std::cout << "Chain height: "
                  << blockchain.size() - 1
                  << "\n";

        std::cout << "UTXO count: "
                  << utxos.size()
                  << "\n";

        std::cout << "Reward TXID: "
                  << coinbase.txid()
                  << "\n";

        std::cout << "Reward address: "
                  << wallet.address()
                  << "\n";

        std::cout
            << "=== BLOCK MINED AND PERSISTED ===\n";

    } catch (const std::exception& e) {
        std::cerr << "Miner error: "
                  << e.what()
                  << "\n";
        return 1;
    }

    return 0;
}
