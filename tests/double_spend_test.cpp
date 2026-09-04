#include "transaction_validator.h"
#include "utxo_set.h"

#include <cassert>
#include <iostream>

int main() {
    larb::UTXOSet utxos;

    const std::string txid =
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    utxos.add({
        {txid, 0},
        {1000, "OWNER"}
    });

    larb::Transaction tx1{};
    tx1.version = 1;
    tx1.inputs.push_back({txid, 0});
    tx1.outputs.push_back({900, "DEST1"});

    larb::Transaction tx2{};
    tx2.version = 1;
    tx2.inputs.push_back({txid, 0});
    tx2.outputs.push_back({900, "DEST2"});

    assert(larb::validate_transaction_value(tx1, utxos));
    assert(larb::apply_transaction(tx1, utxos));

    // UTXO sudah terpakai, transaksi kedua harus ditolak.
    assert(!larb::validate_transaction_value(tx2, utxos));

    std::cout << "Double-spend rejection: PASS\n";
    return 0;
}
