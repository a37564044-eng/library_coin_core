#pragma once

#include "blockchain.h"

#include <optional>
#include <string>

namespace larb {

std::string serialize_chain(const Blockchain& chain);

std::optional<Blockchain> deserialize_chain(
    const std::string& data
);

} // namespace larb
