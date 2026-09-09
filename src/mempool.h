#pragma once

#include "transaction.h"

#include <cstddef>
#include <string>
#include <vector>

namespace larb {

class Mempool {
public:
    bool add(const Transaction& tx);
    bool remove(const std::string& txid);

    const Transaction* find_by_txid(
        const std::string& txid
    ) const;

    std::vector<std::string> find_by_address(
        const std::string& address
    ) const;

    std::size_t size() const;

private:
    std::vector<Transaction> transactions_;
};

} // namespace larb
