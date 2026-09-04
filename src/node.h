#pragma once

#include "block.h"
#include "blockchain.h"
#include "utxo_set.h"

namespace larb {

class Node {
public:
    explicit Node(
        const Block& genesis,
        std::uint32_t difficulty = 3
    );

    bool receive_block(const Block& block);

    /*
     * Adopt chain yang valid dan lebih panjang.
     * Seluruh UTXO dibangun ulang sebelum commit.
     */
    bool adopt_chain(const Blockchain& candidate);
    bool load_state(const std::string& path);
    bool save_state(const std::string& path) const;

    bool is_chain_valid() const;
    std::size_t chain_size() const;

    const Blockchain& blockchain() const;
    const UTXOSet& utxos() const;

private:
    Blockchain blockchain_;
    UTXOSet utxos_;
    std::uint32_t difficulty_;
};

} // namespace larb
