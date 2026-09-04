#include "message_transaction.h"

#include "message.h"
#include "script.h"

namespace larb {

Transaction MessageTransaction::create(
    const std::string& document_id,
    const std::string& message
) {
    Transaction tx{};

    tx.version = 1;

    const auto chunks =
        Message::encode(
            document_id,
            message
        );

    for (const auto& chunk : chunks) {
        TransactionOutput output{};

        output.amount = 0;

        // Simpan PAYLOAD OP_RETURN LENGKAP,
        // termasuk MAGIC + document_id + index + total.
        output.script_pubkey = chunk;

        tx.outputs.push_back(output);
    }

    return tx;
}

} // namespace larb
