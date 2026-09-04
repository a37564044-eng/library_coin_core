#include "blockchain.h"

#include "consensus/pow.h"
#include "consensus/constants.h"
#include "genesis.h"

namespace larb {

Blockchain::Blockchain(
    const Block& genesis,
    std::uint32_t difficulty
)
    : chain_{genesis},
      difficulty_(difficulty) {
}

bool Blockchain::add_block(const Block& block) {
    if (chain_.empty()) {
        return false;
    }

    if (block.header().previous_hash !=
        chain_.back().hash()) {
        return false;
    }

    if (!validate_proof_of_work(
            block,
            difficulty_)) {
        return false;
    }

    chain_.push_back(block);
    return true;
}

bool Blockchain::is_valid() const {
    if (chain_.empty()) {
        return false;
    }

    const Block canonical_genesis = Genesis::create();

    if (chain_[0].hash() != canonical_genesis.hash()) {
        return false;
    }

    if (!validate_proof_of_work(
            chain_[0],
            INITIAL_POW_DIFFICULTY)) {
        return false;
    }

    for (std::size_t i = 1;
         i < chain_.size();
         ++i) {

        const Block& previous = chain_[i - 1];
        const Block& current = chain_[i];

        if (current.header().previous_hash !=
            previous.hash()) {
            return false;
        }

        if (current.header().merkle_root !=
            current.calculate_merkle_root()) {
            return false;
        }

        if (!validate_proof_of_work(
                current,
                difficulty_)) {
            return false;
        }
    }

    return true;
}

const Block& Blockchain::at(std::size_t index) const {
    return chain_.at(index);
}


std::size_t Blockchain::size() const {
    return chain_.size();
}

std::uint32_t Blockchain::difficulty() const {
    return difficulty_;
}

ChainWork Blockchain::chain_work() const {
    ChainWork work = 0;

    if (chain_.size() <= 1) {
        return work;
    }

    const ChainWork work_per_block =
        static_cast<ChainWork>(1) <<
        (4 * difficulty_);

    for (std::size_t i = 1;
         i < chain_.size();
         ++i) {
        work += work_per_block;
    }

    return work;
}

} // namespace larb
