#pragma once

#include "block.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace larb {

using ChainWork = unsigned __int128;

class Blockchain {
public:
    explicit Blockchain(
        const Block& genesis,
        std::uint32_t difficulty = 3
    );

    bool add_block(const Block& block);

    bool is_valid() const;

    std::size_t size() const;

    ChainWork chain_work() const;
    std::uint32_t difficulty() const;

    const Block& at(std::size_t index) const;

private:
    std::vector<Block> chain_;
    std::uint32_t difficulty_;
};

} // namespace larb
