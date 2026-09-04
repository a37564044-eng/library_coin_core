#pragma once

#include "utxo.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace larb {

class UTXOSet {
public:
    void add(const UTXO& utxo);

    bool exists(const OutPoint& outpoint) const;

    const UTXO* find(const OutPoint& outpoint) const;

    bool spend(const OutPoint& outpoint);

    std::size_t size() const;

    void replace_with(const UTXOSet& other);

    std::vector<UTXO> snapshot() const;

private:
    struct OutPointHash {
        std::size_t operator()(const OutPoint& point) const;
    };

    std::unordered_map<
        OutPoint,
        UTXO,
        OutPointHash
    > set_;
};

} // namespace larb
