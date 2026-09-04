#include "chain_codec.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <string>

int main() {
    const std::string data =
        "3\n"
        "1\n" +
        std::to_string(std::numeric_limits<std::size_t>::max()) +
        "\n";

    bool rejected = false;

    try {
        const auto result = larb::deserialize_chain(data);
        rejected = !result.has_value();
    } catch (...) {
        rejected = true;
    }

    assert(rejected);

    std::cout << "Integer overflow hardening: PASS\n";
    return 0;
}
