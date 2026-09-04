#include "src/message_transaction.h"
#include "src/script.h"
#include "src/message.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB MESSAGE TRANSACTION TEST ===\n";

    const std::string document_id = "berita-001";

    const std::string message =
        "LARB adalah ledger digital terdesentralisasi. "
        "Pesan panjang dapat dipecah menjadi beberapa chunk "
        "dan setiap chunk disimpan sebagai payload OP_RETURN.";

    const auto tx =
        larb::MessageTransaction::create(
            document_id,
            message
        );

    if (tx.outputs.empty()) {
        std::cerr << "Transaction outputs: FAIL\n";
        return 1;
    }

    std::cout
        << "Output count: "
        << tx.outputs.size()
        << "\n";

    for (std::size_t i = 0; i < tx.outputs.size(); ++i) {
        if (!larb::Script::is_op_return(
                tx.outputs[i].script_pubkey)) {
            std::cerr << "Output "
                      << i
                      << ": FAIL\n";
            return 1;
        }

        auto payload =
            larb::Script::extract_op_return_data(
                tx.outputs[i].script_pubkey
            );

        auto chunk =
            larb::Message::decode(
                tx.outputs[i].script_pubkey
            );

        std::cout
            << "Chunk "
            << chunk.index + 1
            << "/"
            << chunk.total
            << ": "
            << chunk.data.size()
            << " bytes\n";
    }

    std::cout
        << "MESSAGE -> TRANSACTION: OK\n";

    return 0;
}
