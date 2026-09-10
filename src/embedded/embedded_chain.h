#pragma once

#include <cstddef>
#include <cstdint>

namespace larb {

const std::uint8_t* embedded_chain_data();
std::size_t embedded_chain_size();

bool bootstrap_embedded_chain(const char* path);

}
