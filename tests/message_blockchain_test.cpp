#include "genesis.h"
#include "block.h"
#include "blockchain.h"
#include "consensus/pow.h"
#include "message_transaction.h"
#include "transaction.h"

#include <iostream>
#include <vector>
#include <string>

int main() {
    std::cout << "=== LARB MESSAGE -> TRANSACTION -> BLOCKCHAIN TEST ===\n";

    // 1. Genesis
    larb::Block genesis = larb::Genesis::create();
    larb::Blockchain chain(genesis, 3);

    std::cout << "Genesis chain size: "
              << chain.size() << "\n";

    // 2. Pesan
    const std::string sender = "LARB_USER";

    const std::string message =
        "Ini adalah pesan yang akan dimasukkan "
        "ke dalam transaksi dan kemudian ke block.";

    // 3. Message -> Transaction
    larb::Transaction tx =
        larb::MessageTransaction::create(
            sender,
            message
        );

    std::cout << "Transaction outputs: "
              << tx.outputs.size() << "\n";

    // 4. Serialize transaction
    const std::string tx_data = tx.serialize();

    std::cout << "Transaction bytes: "
              << tx_data.size() << "\n";

    // 5. Transaction -> Block
    std::vector<std::string> transactions;
    transactions.push_back(tx_data);

    std::cout << "Mining block...\n";

    larb::Block block =
        larb::mine_block(
            1,
            genesis.hash(),
            transactions,
            2,
            chain.difficulty()
        );

    std::cout << "Nonce: "
              << block.header().nonce << "\n";

    std::cout << "Block hash: "
              << block.hash() << "\n";

    std::cout << "Merkle root: "
              << block.calculate_merkle_root() << "\n";

    // 6. Pastikan transaksi ada di block
    if (block.transactions().size() != 1) {
        std::cerr << "ERROR: transaction tidak masuk block\n";
        return 1;
    }

    if (block.transactions()[0] != tx_data) {
        std::cerr << "ERROR: transaction bytes berubah\n";
        return 1;
    }

    std::cout << "TRANSACTION -> BLOCK: OK\n";

    // 7. Masukkan block ke blockchain
    if (!chain.add_block(block)) {
        std::cerr << "ERROR: BLOCKCHAIN REJECTED\n";
        return 2;
    }

    // 8. Verifikasi block benar-benar tersimpan
    if (chain.size() != 2) {
        std::cerr << "ERROR: blockchain size bukan 2\n";
        return 1;
    }

    const larb::Block& stored =
        chain.at(1);

    if (stored.transactions().size() != 1) {
        std::cerr << "ERROR: transaction tidak tersimpan\n";
        return 1;
    }

    if (stored.transactions()[0] != tx_data) {
        std::cerr << "ERROR: transaction tersimpan berbeda\n";
        return 1;
    }

    std::cout << "BLOCK -> BLOCKCHAIN: OK\n";
    std::cout << "MESSAGE IS ON BLOCKCHAIN: YES\n";

    return 0;
}
