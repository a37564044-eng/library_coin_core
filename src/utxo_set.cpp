#include "utxo_set.h"

#include <algorithm>

namespace larb {

std::size_t UTXOSet::OutPointHash::operator()(
    const OutPoint& point
) const {
    const std::size_t h1 =
        std::hash<std::string>{}(point.txid);

    const std::size_t h2 =
        std::hash<std::uint32_t>{}(point.output_index);

    return h1 ^ (h2 + 0x9e3779b9 +
                 (h1 << 6) + (h1 >> 2));
}

void UTXOSet::add(const UTXO& utxo) {
    set_[utxo.outpoint] = utxo;
}

bool UTXOSet::exists(
    const OutPoint& outpoint
) const {
    return set_.find(outpoint) != set_.end();
}

const UTXO* UTXOSet::find(
    const OutPoint& outpoint
) const {
    auto it = set_.find(outpoint);

    if (it == set_.end()) {
        return nullptr;
    }

    return &it->second;
}

bool UTXOSet::spend(
    const OutPoint& outpoint
) {
    return set_.erase(outpoint) > 0;
}

std::size_t UTXOSet::size() const {
    return set_.size();
}

void UTXOSet::replace_with(
    const UTXOSet& other
) {
    set_ = other.set_;
}

std::vector<UTXO> UTXOSet::snapshot() const {
    std::vector<UTXO> result;

    result.reserve(set_.size());

    for (const auto& entry : set_) {
        result.push_back(entry.second);
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const UTXO& a, const UTXO& b) {
            if (a.outpoint.txid != b.outpoint.txid) {
                return a.outpoint.txid < b.outpoint.txid;
            }

            return a.outpoint.output_index <
                   b.outpoint.output_index;
        }
    );

    return result;
}

} // namespace larb
