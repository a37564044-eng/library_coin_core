#include "node.h"
#include "transaction_validator.h"
#include "genesis.h"
#include "transaction.h"
#include "script.h"
#include "consensus/constants.h"
#include "consensus/pow.h"
#include "crypto/pqc.h"
#include "address_codec.h"
#include <openssl/sha.h>
#include <vector>

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB FORK REORG TRANSACTION TEST ===\n";

    constexpr std::uint32_t difficulty = 3;

    const larb::Block genesis =
        larb::Genesis::create();

    larb::Node node(genesis, difficulty);

    auto alice_keys = larb::PQC::generate_keypair();

    auto miner_keys = larb::PQC::generate_keypair();

    const std::string miner_script =
        larb::Script::pay_to_pubkey_hash(
            miner_keys.public_key
        );

    unsigned char alice_digest[SHA256_DIGEST_LENGTH];
    SHA256(
        reinterpret_cast<const unsigned char*>(alice_keys.public_key.data()),
        alice_keys.public_key.size(),
        alice_digest
    );

    std::vector<std::uint8_t> alice_payload(
        alice_digest,
        alice_digest + SHA256_DIGEST_LENGTH
    );

    const std::string alice_address =
        larb::AddressCodec::encode(alice_payload);

    const std::string alice_script =
        larb::Script::pay_to_pubkey_hash(
            alice_keys.public_key
        );

    /*
     * =========================================================
     * FORK A
     * Genesis -> A1 -> A2
     *
     * A1 menghasilkan coinbase.
     * A2 membelanjakan coinbase A1 dan membuat UTXO baru.
     * =========================================================
     */

    larb::Transaction a1_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            alice_script,
            1
        );

    larb::Block a1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {a1_coinbase.serialize()},
            1700000001,
            difficulty
        );

    if (!node.receive_block(a1)) {
        std::cerr << "Fork A block 1: FAIL\n";
        return 1;
    }

    const std::string a1_txid =
        a1_coinbase.txid();

    if (!node.utxos().exists({a1_txid, 0})) {
        std::cerr << "Fork A coinbase UTXO: FAIL\n";
        return 1;
    }

    std::cout << "Fork A coinbase UTXO: OK\n";

    /*
     * Spend coinbase A1.
     */
    larb::Transaction a2_spend{};
    a2_spend.version = 1;

    a2_spend.inputs.push_back({
        a1_txid,
        0
    });


      a2_spend.outputs.push_back({
          larb::get_block_reward(1) - 100,
          alice_script
      });

    if (!a2_spend.sign_input(
            0,
            alice_keys.public_key,
            alice_keys.private_key)) {
        std::cerr << "Fork A transaction signing: FAIL\n";
        return 1;
    }


    /*
     * Coinbase A2.
     */
      larb::Transaction a2_coinbase =
          larb::Transaction::coinbase(
              larb::get_block_reward(2),
              miner_script,
              2
          );

    larb::Block a2 =
        larb::mine_block(
            1,
            a1.hash(),
            {
                a2_coinbase.serialize(),
                a2_spend.serialize()
            },
            1700000002,
            difficulty
        );

    std::cout << "A2 spend verify: " << a2_spend.verify_input(0) << "\n";
    std::cout << "A2 merkle valid: " << (a2.header().merkle_root == a2.calculate_merkle_root()) << "\n";
    std::cout << "A2 PoW valid: " << larb::validate_proof_of_work(a2, difficulty) << "\n";
    std::cout << "A2 validate inputs: " << larb::validate_transaction_inputs(a2_spend, node.utxos()) << "\n";
    std::int64_t debug_fee = 0;
    std::cout << "A2 calculate fee: " << larb::calculate_transaction_fee(a2_spend, node.utxos(), debug_fee) << "\n";
    std::cout << "A2 fee: " << debug_fee << "\n";
    std::cout << "A2 value valid: " << larb::validate_transaction_value(a2_spend, node.utxos()) << "\n";

    std::cout << "A2 previous hash match: " << (a2.header().previous_hash == a1.hash()) << "\n";

    if (!node.receive_block(a2)) {
        std::cerr << "Fork A transaction block: FAIL\n";
        return 1;
    }

    const std::string a2_spend_txid =
        a2_spend.txid();

    if (node.utxos().exists({a1_txid, 0})) {
        std::cerr << "Fork A old UTXO spent: FAIL\n";
        return 1;
    }

    if (!node.utxos().exists({a2_spend_txid, 0})) {
        std::cerr << "Fork A spend output: FAIL\n";
        return 1;
    }

    std::cout << "Fork A transaction applied: OK\n";
    std::cout << "Fork A old UTXO spent: OK\n";
    std::cout << "Fork A new UTXO created: OK\n";

    /*
     * =========================================================
     * FORK B
     * Genesis -> B1 -> B2 -> B3
     *
     * Tidak memakai UTXO Fork A.
     * Chainwork lebih tinggi.
     * =========================================================
     */

    larb::Transaction b1_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(1),
            alice_address,
            1
        );

    larb::Block b1 =
        larb::mine_block(
            1,
            genesis.hash(),
            {b1_coinbase.serialize()},
            1700000011,
            difficulty
        );

    larb::Transaction b2_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(2),
            alice_address,
            2
        );

    larb::Block b2 =
        larb::mine_block(
            1,
            b1.hash(),
            {b2_coinbase.serialize()},
            1700000012,
            difficulty
        );

    larb::Transaction b3_coinbase =
        larb::Transaction::coinbase(
            larb::get_block_reward(3),
            alice_address,
            3
        );

    larb::Block b3 =
        larb::mine_block(
            1,
            b2.hash(),
            {b3_coinbase.serialize()},
            1700000013,
            difficulty
        );

    larb::Blockchain winning_chain(
        genesis,
        difficulty
    );

    if (!winning_chain.add_block(b1) ||
        !winning_chain.add_block(b2) ||
        !winning_chain.add_block(b3)) {
        std::cerr << "Construct winning fork: FAIL\n";
        return 1;
    }

    std::cout << "Competing transaction-free fork constructed: OK\n";

    if (winning_chain.chain_work() <=
        node.blockchain().chain_work()) {
        std::cerr << "Higher chainwork detection: FAIL\n";
        return 1;
    }

    std::cout << "Higher chainwork detected: OK\n";

    /*
     * =========================================================
     * REORG
     * =========================================================
     */

    if (!node.adopt_chain(winning_chain)) {
        std::cerr << "Winning fork adoption: FAIL\n";
        return 1;
    }

    std::cout << "Winning fork adopted: OK\n";

    if (node.blockchain().at(
            node.chain_size() - 1
        ).hash() != b3.hash()) {
        std::cerr << "Tip switched to winning fork: FAIL\n";
        return 1;
    }

    std::cout << "Tip switched to winning fork: OK\n";

    if (!node.is_chain_valid()) {
        std::cerr << "Winning chain validation: FAIL\n";
        return 1;
    }

    std::cout << "Winning chain validation: OK\n";

    /*
     * =========================================================
     * PASTIKAN STATE FORK A HILANG
     * =========================================================
     */

    if (node.utxos().exists({a1_txid, 0})) {
        std::cerr << "Old Fork A coinbase UTXO removed: FAIL\n";
        return 1;
    }

    if (node.utxos().exists({a2_spend_txid, 0})) {
        std::cerr << "Old Fork A transaction UTXO removed: FAIL\n";
        return 1;
    }

    std::cout << "Old Fork A coinbase UTXO removed: OK\n";
    std::cout << "Old Fork A transaction UTXO removed: OK\n";

    /*
     * =========================================================
     * PASTIKAN STATE FORK B ADA
     * =========================================================
     */

    const std::string b1_txid =
        b1_coinbase.txid();

    const std::string b2_txid =
        b2_coinbase.txid();

   const std::string b3_txid =
        b3_coinbase.txid();

    if (!node.utxos().exists({b1_txid, 0})) {
        std::cerr << "Winning B1 UTXO: FAIL\n";
        return 1;
    }

    if (!node.utxos().exists({b2_txid, 0})) {
        std::cerr << "Winning B2 UTXO: FAIL\n";
        return 1;
    }

    if (!node.utxos().exists({b3_txid, 0})) {
        std::cerr << "Winning B3 UTXO: FAIL\n";
        return 1;
    }

    std::cout << "Winning B1 UTXO rebuilt: OK\n";
    std::cout << "Winning B2 UTXO rebuilt: OK\n";
    std::cout << "Winning B3 UTXO rebuilt: OK\n";

    if (node.chain_size() != 4) {
        std::cerr << "Final chain size: FAIL\n";
        return 1;
    }

    std::cout << "Final chain size: OK\n";

    std::cout
        << "=== ALL FORK REORG TRANSACTION TESTS PASSED ===\n";

    return 0;
}
