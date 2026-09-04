#include "constants.h"

namespace larb {

std::int64_t get_block_reward(std::uint64_t height) {
    const std::uint64_t halvings =
        height / HALVING_INTERVAL;

    // Setelah 63 kali halving, reward habis.
    if (halvings >= 63) {
        return 0;
    }

    return INITIAL_BLOCK_REWARD >> halvings;
}

} // namespace larb
