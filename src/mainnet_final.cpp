#include "mainnet_final.h"

#include "genesis.h"
#include "block_validator.h"
#include "utxo_set.h"

namespace larb {

bool MainnetFinal::verify() {
    const Block genesis = Genesis::create();

    if (genesis.hash() != GENESIS_HASH)
        return false;

    if (genesis.header().nonce != GENESIS_NONCE)
        return false;

    UTXOSet utxos;

    // Genesis is validated at height 0.
    // The normal block validator intentionally excludes
    // genesis from the spendable UTXO set.
    if (!validate_block(genesis, 0, utxos))
        return false;

    return true;
}

} // namespace larb
