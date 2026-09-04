#include "src/message.h"

#include <iostream>
#include <string>
#include <vector>

int main() {
    std::cout << "=== LARB MESSAGE TEST ===\n";

    const std::string document_id = "document-001";

    const std::string original =
        "LARB DIGITAL BOOK. "
        "Ini adalah dokumen panjang yang akan dipecah "
        "menjadi beberapa chunk OP_RETURN. "
        "Setiap chunk memiliki metadata sendiri, "
        "kemudian semua chunk akan didecode dan "
        "disusun kembali menjadi dokumen asli. "
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
        "0123456789 ";

    std::vector<larb::MessageChunk> chunks;

    try {
        const auto encoded =
            larb::Message::encode(
                document_id,
                original
            );

        std::cout
            << "Chunk count: "
            << encoded.size()
            << "\n";

        for (const auto& payload : encoded) {
            chunks.push_back(
                larb::Message::decode(payload)
            );
        }

        const std::string reconstructed =
            larb::Message::assemble(chunks);

        if (reconstructed != original) {
            std::cerr
                << "MESSAGE ROUND-TRIP: FAIL\n";
            return 1;
        }

        std::cout << "Encode: OK\n";
        std::cout << "Decode: OK\n";
        std::cout << "Assemble: OK\n";
        std::cout
            << "Original bytes: "
            << original.size()
            << "\n";
        std::cout
            << "Reconstructed bytes: "
            << reconstructed.size()
            << "\n";

        std::cout
            << "MESSAGE ROUND-TRIP: OK\n";

    } catch (const std::exception& e) {
        std::cerr
            << "ERROR: "
            << e.what()
            << "\n";
        return 1;
    }

    return 0;
}
