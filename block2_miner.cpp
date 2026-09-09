#include "src/blockchain.h"
#include "src/block_validator.h"
#include "src/consensus/constants.h"
#include "src/consensus/pow.h"
#include "src/genesis.h"
#include "src/message.h"
#include "src/persistence.h"
#include "src/transaction.h"
#include "src/transaction_validator.h"
#include "src/utxo_set.h"
#include "src/wallet/wallet.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

const std::string BLOCK1_TXID =
    "210ba3cc9974e57a82e5b153f77e2417b95b56bcbf2d61eb27170c6c7cef49cf";

const std::string MESSAGE =
    "Untuk Mbak Kharisa, kita mungkin terpisah oleh jarak antara Kampus 1 dan Kampus 2 SMK Muhammadiyah 1 Playen. Namun, AXIOO dan Library Coin akan menjadi saksi kecil perjalanan ini—menyimpan sebuah catatan tentang waktu ketika kita pernah berada di halaman yang sama.";

const std::string DOCUMENT_ID = "BLOCK2-KHARISA";

}

int main() {
    std::cout << "=== LARB BLOCK #2 MINER ===\n";

    const std::string wallet_path = "/public/.larb/wallet.dat";
    const std::string chain_path = "larb_chain.dat";

    try {
        // ------------------------------------------------------------
        // 1. Load wallet
        // ------------------------------------------------------------
        std::string password;
        std::cout << "Wallet password: ";
        std::getline(std::cin, password);

        larb::Wallet wallet =
            larb::Wallet::load(wallet_path, password);

        const std::string address = wallet.address();

        std::cout << "Wallet load: OK\n";
        std::cout << "Address: " << address << "\n";

        // ------------------------------------------------------------
        // 2. Load existing chain
        // ------------------------------------------------------------
        const larb::Block genesis = larb::Genesis::create();

        larb::Blockchain blockchain(
            genesis,
            larb::INITIAL_POW_DIFFICULTY
        );

        larb::UTXOSet utxos;

        if (!larb::Persistence::load(
                chain_path,
                blockchain,
                utxos)) {
            std::cerr << "Blockchain load: FAIL\n";
            return 1;
        }

        if (!blockchain.is_valid()) {
            std::cerr << "Blockchain validation: FAIL\n";
            return 1;
        }

        std::cout << "Blockchain load: OK\n";
        std::cout << "Current height: "
                  << blockchain.size() - 1
                  << "\n";

        // ------------------------------------------------------------
        // 3. We expect Block #1 exactly
        // ------------------------------------------------------------
        if (blockchain.size() != 2) {
            std::cerr
                << "Expected chain height 1 before Block #2\n";
            return 1;
        }

        const larb::Block& block1 =
            blockchain.at(1);

        if (!larb::validate_proof_of_work(
                block1,
                blockchain.difficulty())) {
            std::cerr << "Block #1 PoW validation: FAIL\n";
            return 1;
        }

        std::cout << "Block #1 PoW: OK\n";
        std::cout << "Block #1 hash: "
                  << block1.hash()
                  << "\n";

        // ------------------------------------------------------------
        // 4. Confirm Block #1 UTXO
        // ------------------------------------------------------------
        larb::OutPoint outpoint{
            BLOCK1_TXID,
            0
        };

        const larb::UTXO* source =
            utxos.find(outpoint);

        if (source == nullptr) {
            std::cerr
                << "Block #1 UTXO: NOT FOUND\n";
            return 1;
        }

        constexpr std::int64_t INPUT_AMOUNT =
            5'000'000'000;

        constexpr std::int64_t OUTPUT_AMOUNT =
            4'900'000'000;

        constexpr std::int64_t FEE =
            100'000'000;

        if (source->output.amount != INPUT_AMOUNT) {
            std::cerr
                << "Block #1 UTXO amount mismatch\n";
            return 1;
        }

        if (source->output.script_pubkey != address) {
            std::cerr
                << "Block #1 UTXO does not belong to wallet\n";
            return 1;
        }

        std::cout << "Block #1 UTXO: OK\n";
        std::cout << "Input: 50 LARB\n";

        // ------------------------------------------------------------
        // 5. Build spending transaction
        // ------------------------------------------------------------
        larb::Transaction tx;

        tx.version = 1;

        tx.inputs.push_back(
            larb::TransactionInput{
                BLOCK1_TXID,
                0,
                "",
                ""
            }
        );

        tx.outputs.push_back(
            larb::TransactionOutput{
                OUTPUT_AMOUNT,
                address
            }
        );

        // Message::encode() performs the required <=80-byte
        // OP_RETURN chunking without changing the original message.
        const std::vector<std::string> chunks =
            larb::Message::encode(
                DOCUMENT_ID,
                MESSAGE
            );

        for (const std::string& chunk : chunks) {
            tx.outputs.push_back(
                larb::TransactionOutput{
                    0,
                    chunk
                }
            );
        }

        std::cout << "Message chunks: "
                  << chunks.size()
                  << "\n";

        // ------------------------------------------------------------
        // 6. Sign transaction with wallet key
        // ------------------------------------------------------------
        const std::string public_key(
            reinterpret_cast<const char*>(wallet.public_key().data()),
            wallet.public_key().size()
        );

        const std::string private_key(
            reinterpret_cast<const char*>(wallet.secret_key().data()),
            wallet.secret_key().size()
        );

        if (!tx.sign_input(
                0,
                public_key,
                private_key)) {
            std::cerr
                << "Transaction signing: FAIL\n";
            return 1;
        }

        std::cout << "Transaction signing: OK\n";

        // ------------------------------------------------------------
        // 7. Validate transaction
        // ------------------------------------------------------------
        larb::UTXOSet working_utxos = utxos;

        if (!larb::validate_transaction_inputs(
                tx,
                working_utxos)) {
            std::cerr
                << "Transaction input validation: FAIL\n";
            return 1;
        }

        if (!larb::validate_transaction_value(
                tx,
                working_utxos)) {
            std::cerr
                << "Transaction value validation: FAIL\n";
            return 1;
        }

        const std::int64_t input_total =
            INPUT_AMOUNT;

        const std::int64_t output_total =
            OUTPUT_AMOUNT;

        const std::int64_t fee =
            input_total - output_total;

        if (fee != FEE) {
            std::cerr
                << "Transaction fee mismatch\n";
            return 1;
        }

        std::cout << "Transaction validation: OK\n";
        std::cout << "Transaction fee: "
                  << (fee / larb::COIN)
                  << " LARB\n";

        std::cout << "Transaction TXID: "
                  << tx.txid()
                  << "\n";

        // ------------------------------------------------------------
        // 8. Build Block #2 coinbase
        // ------------------------------------------------------------
        constexpr std::uint64_t BLOCK2_HEIGHT = 2;

        const std::int64_t subsidy =
            larb::get_block_reward(BLOCK2_HEIGHT);

        const std::int64_t coinbase_amount =
            subsidy + fee;

        const larb::Transaction coinbase =
            larb::Transaction::coinbase(
                coinbase_amount,
                address,
                BLOCK2_HEIGHT
            );

        // ------------------------------------------------------------
        // 9. Serialize transactions
        // ------------------------------------------------------------
        const std::string coinbase_data =
            coinbase.serialize();

        const std::string tx_data =
            tx.serialize();

        std::cout
            << "\n=== MINING BLOCK #2 ===\n";

        std::cout << "Height: "
                  << BLOCK2_HEIGHT
                  << "\n";

        std::cout << "Difficulty: "
                  << blockchain.difficulty()
                  << "\n";

        std::cout << "Coinbase reward: "
                  << (coinbase_amount / larb::COIN)
                  << " LARB\n";

        // ------------------------------------------------------------
        // 10. Mine Block #2
        // ------------------------------------------------------------
        const std::uint64_t timestamp =
            static_cast<std::uint64_t>(
                std::time(nullptr)
            );

        const larb::Block block2 =
            larb::mine_block(
                1,
                block1.hash(),
                {
                    coinbase_data,
                    tx_data
                },
                timestamp,
                blockchain.difficulty()
            );

        std::cout << "Block #2 nonce: "
                  << block2.header().nonce
                  << "\n";

        std::cout << "Block #2 hash: "
                  << block2.hash()
                  << "\n";

        // ------------------------------------------------------------
        // 11. Validate Block #2 completely
        // ------------------------------------------------------------
        larb::UTXOSet block_utxos = utxos;

        if (!larb::validate_block(
                block2,
                BLOCK2_HEIGHT,
                block_utxos)) {
            std::cerr
                << "Block #2 validation: FAIL\n";
            return 1;
        }

        std::cout << "Block #2 validation: OK\n";

        // ------------------------------------------------------------
        // 12. Add and persist Block #2
        // ------------------------------------------------------------
        if (!blockchain.add_block(block2)) {
            std::cerr
                << "Block #2 insertion: FAIL\n";
            return 1;
        }

        utxos.replace_with(block_utxos);

        if (!larb::Persistence::save(
                "larb_chain.dat",
                blockchain,
                utxos)) {
            std::cerr
                << "Blockchain save: FAIL\n";
            return 1;
        }

        std::cout
            << "Blockchain save: OK\n";

        std::cout
            << "\n=== BLOCK #2 COMPLETE ===\n";

        std::cout
            << "Chain height: "
            << blockchain.size() - 1
            << "\n";

        std::cout
            << "Block #2 hash: "
            << block2.hash()
            << "\n";

        std::cout
            << "Transaction TXID: "
            << tx.txid()
            << "\n";

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr
            << "ERROR: "
            << e.what()
            << "\n";
        return 1;
    }
}
