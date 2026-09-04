#include "genesis.h"
#include "consensus/constants.h"
#include "consensus/pow.h"
#include "transaction.h"

#include <iostream>
#include <vector>

int main() {
    larb::Transaction tx{};
    tx.version = 1;

    tx.outputs.push_back(
        larb::TransactionOutput::op_return(
            larb::Genesis::MESSAGE
        )
    );

    std::vector<std::string> transactions = {
        tx.serialize()
    };

    const larb::Block genesis = larb::mine_block(
        1,
        "0",
        transactions,
        larb::Genesis::TIMESTAMP,
        larb::INITIAL_POW_DIFFICULTY
    );

    std::cout << "Nonce: "
              << genesis.header().nonce << "\n";

    std::cout << "Merkle root: "
              << genesis.header().merkle_root << "\n";

    std::cout << "Genesis hash: "
              << genesis.hash() << "\n";

    return 0;
}
