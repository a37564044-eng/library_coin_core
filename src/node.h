#pragma once

#include "block.h"
#include "blockchain.h"
#include "mempool.h"
#include "utxo_set.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace larb {

class Node {
public:
    explicit Node(
        const Block& genesis,
        std::uint32_t difficulty = 3
    );

    bool receive_block(const Block& block);

    /*
     * Add transaksi ke mempool.
     * Transaksi belum masuk blockchain sampai dimasukkan ke block.
     */
    bool submit_transaction(const Transaction& tx);

    bool remove_transaction(const std::string& txid);

    const Transaction* find_mempool_tx(
        const std::string& txid
    ) const;

    std::vector<std::string> find_mempool_by_address(
        const std::string& address
    ) const;

    std::size_t mempool_size() const;

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
    Mempool mempool_;
    std::uint32_t difficulty_;
};

} // namespace larb
