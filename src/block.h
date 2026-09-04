#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace larb {

struct BlockHeader {
    std::uint32_t version;
    std::string previous_hash;
    std::string merkle_root;
    std::uint64_t timestamp;
    std::uint64_t nonce;
};

class Block {
public:
    Block();

    Block(
        std::uint32_t version,
        const std::string& previous_hash,
        const std::vector<std::string>& transactions,
        std::uint64_t timestamp,
        std::uint64_t nonce
    );

    const BlockHeader& header() const;
    const std::vector<std::string>& transactions() const;

    std::string calculate_merkle_root() const;
    std::string serialize_header() const;
    std::string hash() const;

private:
    BlockHeader header_;
    std::vector<std::string> transactions_;
};

} // namespace larb
