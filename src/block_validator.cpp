#include "block_validator.h"

#include "consensus/constants.h"
#include "transaction.h"
#include "transaction_validator.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <string>

namespace larb {

bool validate_block(
    const Block& block,
    std::uint64_t height,
    UTXOSet& utxos
) {
    /*
     * Timestamp block tidak boleh terlalu jauh
     * ke masa depan.
     */
    const std::uint64_t now =
        static_cast<std::uint64_t>(
            std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now()
            )
        );

    constexpr std::uint64_t MAX_FUTURE_DRIFT =
        2 * 60 * 60;

    if (block.header().timestamp >
        now + MAX_FUTURE_DRIFT) {
        return false;
    }

    const auto& transactions =
        block.transactions();

    /*
     * Merkle root harus selalu cocok.
     */
    if (block.header().merkle_root !=
        block.calculate_merkle_root()) {
        return false;
    }

    /*
     * Genesis adalah artefak.
     */
    if (height == 0) {
        return true;
    }

    /*
     * Block non-genesis wajib punya coinbase.
     */
    if (transactions.empty()) {
        return false;
    }

    Transaction coinbase;

    try {
        coinbase =
            Transaction::deserialize(
                transactions[0]
            );
    } catch (...) {
        return false;
    }

    const std::string expected_coinbase_data =
        "LARB-COINBASE-HEIGHT:" + std::to_string(height);

    if (coinbase.coinbase_data != expected_coinbase_data) {
        return false;
    }

    const std::int64_t reward =
        get_block_reward(height);

    if (reward <= 0 ||
        reward > MAX_MONEY) {
        return false;
    }

    /*
     * Semua perubahan dilakukan terhadap copy.
     */
    UTXOSet working_utxos = utxos;

    /*
     * Total fee seluruh transaksi biasa.
     */
    std::int64_t total_fees = 0;

    /*
     * Validasi dan apply semua transaksi setelah
     * coinbase.
     */
    for (std::size_t i = 1;
         i < transactions.size();
         ++i) {

        Transaction tx;

        try {
            tx =
                Transaction::deserialize(
                    transactions[i]
                );
        } catch (...) {
            return false;
        }

        /*
         * Coinbase kedua dilarang.
         */
        if (tx.inputs.empty()) {
            return false;
        }

        /*
         * Hitung fee menggunakan UTXO state
         * sebelum transaksi diterapkan.
         */
        std::int64_t fee = 0;

        if (!calculate_transaction_fee(
                tx,
                working_utxos,
                fee)) {
            return false;
        }

        /*
         * Lindungi total fee dari overflow.
         */
        if (fee < 0 ||
            total_fees >
                INT64_MAX - fee) {
            return false;
        }

        total_fees += fee;

        /*
         * Terapkan transaksi setelah fee valid.
         */
        if (!apply_transaction(
                tx,
                working_utxos)) {
            return false;
        }
    }

    /*
     * Coinbase maksimum:
     * block reward + seluruh transaction fees.
     */
    if (total_fees >
        INT64_MAX - reward) {
        return false;
    }

    const std::int64_t allowed_coinbase =
        reward + total_fees;

    /*
     * Coinbase harus valid secara struktur.
     */
    if (coinbase.inputs.size() != 0 ||
        coinbase.outputs.empty()) {
        return false;
    }

    std::int64_t coinbase_value = 0;

    for (const auto& output : coinbase.outputs) {
        if (output.amount < 0 ||
            output.amount > MAX_MONEY) {
            return false;
        }

        if (coinbase_value >
            INT64_MAX - output.amount) {
            return false;
        }

        coinbase_value += output.amount;

        if (coinbase_value > MAX_MONEY) {
            return false;
        }
    }

    if (coinbase_value > allowed_coinbase) {
        return false;
    }

    /*
     * Coinbase harus minimal sebesar block reward.
     */
    if (coinbase_value < reward) {
        return false;
    }

    /*
     * Masukkan coinbase ke working UTXO set.
     */
    const std::string txid =
        coinbase.txid();

    for (std::size_t i = 0;
         i < coinbase.outputs.size();
         ++i) {

        working_utxos.add(
            UTXO{
                OutPoint{
                    txid,
                    static_cast<std::uint32_t>(i)
                },
                coinbase.outputs[i]
            }
        );
    }

    /*
     * Commit hanya setelah seluruh block valid.
     */
    utxos.replace_with(working_utxos);

    return true;
}

} // namespace larb
