#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace larb {

struct MessageChunk {
    std::string document_id;
    std::uint32_t index;
    std::uint32_t total;
    std::string data;
};

class Message {
public:
    static constexpr std::size_t OP_RETURN_LIMIT = 80;

    // Membagi pesan panjang menjadi beberapa payload OP_RETURN.
    static std::vector<std::string> encode(
        const std::string& document_id,
        const std::string& message
    );

    // Mengambil kembali satu chunk dari OP_RETURN.
    static MessageChunk decode(
        const std::string& payload
    );

    // Menggabungkan chunk menjadi pesan asli.
    static std::string assemble(
        std::vector<MessageChunk> chunks
    );
};

} // namespace larb
