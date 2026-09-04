#pragma once

#include "transaction.h"
#include "utxo_set.h"

namespace larb {

bool validate_transaction_inputs(
    const Transaction& tx,
    const UTXOSet& utxos
);

bool validate_transaction_value(
    const Transaction& tx,
    const UTXOSet& utxos
);

bool validate_coinbase(
    const Transaction& tx,
    std::int64_t block_reward
);

bool apply_coinbase(
    const Transaction& tx,
    std::int64_t block_reward,
    UTXOSet& utxos
);

bool apply_transaction(
    const Transaction& tx,
    UTXOSet& utxos
);


bool calculate_transaction_fee(
    const Transaction& tx,
    const UTXOSet& utxos,
    std::int64_t& fee
);

} // namespace larb
