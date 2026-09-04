#include "genesis.h"

#include "transaction.h"

namespace larb {

Block Genesis::create() {
    Transaction tx{};
    tx.version = 1;

    tx.outputs.push_back(
        TransactionOutput::op_return(MESSAGE)
    );

    const std::string serialized_tx =
        tx.serialize();

    return Block(
        1,
        "0",
        {serialized_tx},
        TIMESTAMP,
        NONCE
    );
}

} // namespace larb
