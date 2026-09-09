#include "mempool.h"

#include <algorithm>

namespace larb {

bool Mempool::add(const Transaction& tx) {
    if (find_by_txid(tx.txid()) != nullptr)
        return false;

    transactions_.push_back(tx);
    return true;
}

bool Mempool::remove(const std::string& txid) {
    const auto old_size = transactions_.size();

    transactions_.erase(
        std::remove_if(
            transactions_.begin(),
            transactions_.end(),
            [&](const Transaction& tx) {
                return tx.txid() == txid;
            }
        ),
        transactions_.end()
    );

    return transactions_.size() != old_size;
}

const Transaction* Mempool::find_by_txid(
    const std::string& txid
) const {
    for (const auto& tx : transactions_) {
        if (tx.txid() == txid)
            return &tx;
    }

    return nullptr;
}

std::vector<std::string> Mempool::find_by_address(
    const std::string& address
) const {
    std::vector<std::string> result;

    for (const auto& tx : transactions_) {
        for (const auto& output : tx.outputs) {
            if (output.script_pubkey == address) {
                result.push_back(tx.txid());
                break;
            }
        }
    }

    return result;
}

std::size_t Mempool::size() const {
    return transactions_.size();
}

} // namespace larb
