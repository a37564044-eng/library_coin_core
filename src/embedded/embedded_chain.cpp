#include "embedded_chain.h"

#include "larb_chain_embedded.h"

#include <fstream>

namespace larb {

const std::uint8_t* embedded_chain_data() {
    return larb_embedded_chain;
}

std::size_t embedded_chain_size() {
    return larb_embedded_chain_size;
}

bool bootstrap_embedded_chain(const char* path) {
    std::ifstream check(path, std::ios::binary);

    if (check.good()) {
        return true;
    }

    std::ofstream out(path, std::ios::binary);

    if (!out) {
        return false;
    }

    out.write(
        reinterpret_cast<const char*>(larb_embedded_chain),
        static_cast<std::streamsize>(larb_embedded_chain_size)
    );

    return out.good();
}

}
