#include "block.h"

#include <openssl/sha.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace larb {

namespace {

std::string sha256_hex(const std::string& data) {
    unsigned char digest[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(data.data()),
        data.size(),
        digest
    );

    std::ostringstream out;

    for (unsigned char byte : digest) {
        out << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(byte);
    }

    return out.str();
}

} // namespace

Block::Block()
    : header_{1, "", "", 0, 0},
      transactions_{} {
}

Block::Block(
    std::uint32_t version,
    const std::string& previous_hash,
    const std::vector<std::string>& transactions,
    std::uint64_t timestamp,
    std::uint64_t nonce
)
    : header_{
          version,
          previous_hash,
          "",
          timestamp,
          nonce
      },
      transactions_(transactions) {

    header_.merkle_root = calculate_merkle_root();
}

const BlockHeader& Block::header() const {
    return header_;
}

const std::vector<std::string>& Block::transactions() const {
    return transactions_;
}

std::string Block::calculate_merkle_root() const {
    if (transactions_.empty()) {
        return sha256_hex("");
    }

    std::vector<std::string> level;

    for (const auto& tx : transactions_) {
        level.push_back(sha256_hex(tx));
    }

    while (level.size() > 1) {
        std::vector<std::string> next;

        for (std::size_t i = 0; i < level.size(); i += 2) {
            const std::string& left = level[i];

            const std::string& right =
                (i + 1 < level.size())
                    ? level[i + 1]
                    : level[i];

            next.push_back(sha256_hex(left + right));
        }

        level = std::move(next);
    }

    return level[0];
}

std::string Block::serialize_header() const {
    std::ostringstream out;

    out << header_.version << '|'
        << header_.previous_hash << '|'
        << header_.merkle_root << '|'
        << header_.timestamp << '|'
        << header_.nonce;

    return out.str();
}

std::string Block::hash() const {
    return sha256_hex(serialize_header());
}

} // namespace larb
