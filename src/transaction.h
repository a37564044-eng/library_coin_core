#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace larb {

struct TransactionInput {
    std::string previous_txid;
    std::uint32_t output_index;

    // ML-DSA-44 public key dan signature.
    std::string public_key;
    std::string signature;
};

struct TransactionOutput {
    std::int64_t amount;
    std::string script_pubkey;

    static TransactionOutput op_return(
        const std::string& data
    );
};

struct Transaction {
    static Transaction coinbase(
        std::int64_t reward,
        const std::string& script_pubkey
    );

    static Transaction coinbase(
        std::int64_t reward,
        const std::string& script_pubkey,
        std::uint64_t height
    );

    std::uint32_t version;
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;

    /*
     * Metadata khusus coinbase.
     *
     * Kosong = format transaksi lama.
     * Terisi = coinbase dengan metadata height.
     */
    std::string coinbase_data;

    std::string serialize() const;

    // Data yang ditandatangani input.
    std::string serialize_for_signature(
        std::size_t input_index
    ) const;

    static Transaction deserialize(
        const std::string& data
    );

    std::string txid() const;

    bool sign_input(
        std::size_t input_index,
        const std::string& public_key,
        const std::string& private_key
    );

    bool verify_input(
        std::size_t input_index
    ) const;
};

} // namespace larb
