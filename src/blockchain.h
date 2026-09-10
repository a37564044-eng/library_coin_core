#pragma once

#include "block.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace larb {

#if defined(_MSC_VER) && !defined(__clang__)
struct ChainWork {
    std::uint64_t lo = 0;
    std::uint64_t hi = 0;

    constexpr ChainWork() = default;
    constexpr ChainWork(std::uint64_t value) : lo(value), hi(0) {}

    ChainWork& operator+=(const ChainWork& other) {
        const std::uint64_t old_lo = lo;
        lo += other.lo;
        hi += other.hi + (lo < old_lo ? 1 : 0);
        return *this;
    }

    friend ChainWork operator+(ChainWork a, const ChainWork& b) {
        a += b;
        return a;
    }

    friend bool operator>(const ChainWork& a, const ChainWork& b) {
        return (a.hi > b.hi) || (a.hi == b.hi && a.lo > b.lo);
    }

      friend bool operator<=(const ChainWork& a, const ChainWork& b) {
          return !(a > b);
      }

      friend bool operator<=(const ChainWork& a, const ChainWork& b) {
          return !(a > b);
      }

    friend bool operator==(const ChainWork& a, const ChainWork& b) {
        return a.hi == b.hi && a.lo == b.lo;
    }

    friend bool operator!=(const ChainWork& a, const ChainWork& b) {
        return !(a == b);
    }

    static ChainWork one_shifted(std::uint32_t shift) {
        ChainWork result;
        if (shift < 64) {
            result.lo = std::uint64_t(1) << shift;
        } else if (shift < 128) {
            result.hi = std::uint64_t(1) << (shift - 64);
        }
        return result;
    }
};
#else
using ChainWork = unsigned __int128;
#endif

ChainWork chain_work_one_shifted(std::uint32_t shift);

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
