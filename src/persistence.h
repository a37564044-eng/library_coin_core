#pragma once

#include "blockchain.h"
#include "utxo_set.h"

#include <string>

namespace larb {

class Persistence {
public:
    static bool save(
        const std::string& path,
        const Blockchain& chain,
        const UTXOSet& utxos
    );

    static bool load(
        const std::string& path,
        Blockchain& chain,
        UTXOSet& utxos
    );
};

} // namespace larb
