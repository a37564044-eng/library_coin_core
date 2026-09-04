#include "transaction.h"
#include "script.h"
#include "crypto/pqc.h"

#include <openssl/sha.h>

#include <iomanip>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace larb {

TransactionOutput TransactionOutput::op_return(
    const std::string& data
) {
    return {
        0,
        Script::op_return(data)
    };
}

static void append_u32(
    std::string& out,
    std::uint32_t value
) {
    out.append(
        reinterpret_cast<const char*>(&value),
        sizeof(value)
    );
}

static void append_i64(
    std::string& out,
    std::int64_t value
) {
    out.append(
        reinterpret_cast<const char*>(&value),
        sizeof(value)
    );
}

static void append_u64(
    std::string& out,
    std::uint64_t value
) {
    out.append(
        reinterpret_cast<const char*>(&value),
        sizeof(value)
    );
}

static void append_string(
    std::string& out,
    const std::string& value
) {
    const std::uint64_t size =
        static_cast<std::uint64_t>(value.size());

    append_u64(out, size);
    out.append(value);
}

Transaction Transaction::coinbase(
    std::int64_t reward,
    const std::string& script_pubkey
) {
    Transaction tx{};
    tx.version = 1;
    tx.outputs.push_back({reward, script_pubkey});
    return tx;
}

Transaction Transaction::coinbase(
    std::int64_t reward,
    const std::string& script_pubkey,
    std::uint64_t height
) {
    Transaction tx{};
    tx.version = 1;
    tx.outputs.push_back({reward, script_pubkey});
    tx.coinbase_data =
        "LARB-COINBASE-HEIGHT:" + std::to_string(height);
    return tx;
}

std::string Transaction::serialize() const {
    std::string out;

    append_u32(out, version);

    append_u64(
        out,
        static_cast<std::uint64_t>(inputs.size())
    );

    for (const auto& input : inputs) {
        append_string(out, input.previous_txid);
        append_u32(out, input.output_index);
        append_string(out, input.public_key);
        append_string(out, input.signature);
    }

    append_u64(
        out,
        static_cast<std::uint64_t>(outputs.size())
    );

    for (const auto& output : outputs) {
        append_i64(out, output.amount);
        append_string(out, output.script_pubkey);
    }

    if (!coinbase_data.empty()) {
        append_string(out, "LARB-CB1");
        append_string(out, coinbase_data);
    }
    return out;
}

std::string Transaction::serialize_for_signature(
    std::size_t input_index
) const {
    if (input_index >= inputs.size()) {
        throw std::out_of_range(
            "transaction input index out of range"
        );
    }

    std::string out;

    append_u32(out, version);

    append_u64(
        out,
        static_cast<std::uint64_t>(inputs.size())
    );

    for (std::size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i];

        append_string(out, input.previous_txid);
        append_u32(out, input.output_index);

        /*
         * Signature preimage harus stabil.
         * Public key dan signature TIDAK pernah
         * dimasukkan untuk input mana pun.
         * Dengan demikian signature input lain
         * tidak dapat mengubah message yang diverifikasi.
         */
        append_string(out, "");
        append_string(out, "");
    }

    append_u64(
        out,
        static_cast<std::uint64_t>(outputs.size())
    );

    for (const auto& output : outputs) {
        append_i64(out, output.amount);
        append_string(out, output.script_pubkey);
    }

    return out;
}

static std::string sha256_hex(
    const std::string& data
) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(
            data.data()
        ),
        data.size(),
        hash
    );

    std::ostringstream out;

    for (unsigned char byte : hash) {
        out << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(byte);
    }

    return out.str();
}

std::string Transaction::txid() const {
    return sha256_hex(serialize());
}

bool Transaction::sign_input(
    std::size_t input_index,
    const std::string& public_key,
    const std::string& private_key
) {
    if (input_index >= inputs.size())
        return false;

    if (public_key.empty() || private_key.empty())
        return false;

    try {
        const std::string signature =
            PQC::sign(
                serialize_for_signature(input_index),
                private_key
            );

        inputs[input_index].public_key = public_key;
        inputs[input_index].signature = signature;

        return true;
    }
    catch (...) {
        return false;
    }
}

bool Transaction::verify_input(
    std::size_t input_index
) const {
    if (input_index >= inputs.size())
        return false;

    const auto& input = inputs[input_index];

    if (input.public_key.empty() ||
        input.signature.empty())
        return false;

    try {
        return PQC::verify(
            serialize_for_signature(input_index),
            input.signature,
            input.public_key
        );
    }
    catch (...) {
        return false;
    }
}

} // namespace larb

namespace {

template <typename T>
T read_binary(
    const std::string& data,
    std::size_t& pos
) {
    if (pos > data.size() ||
        sizeof(T) > data.size() - pos) {
        throw std::runtime_error(
            "transaction truncated"
        );
    }

    T value{};

    std::memcpy(
        &value,
        data.data() + pos,
        sizeof(T)
    );

    pos += sizeof(T);
    return value;
}

std::string read_binary_string(
    const std::string& data,
    std::size_t& pos
) {
    const std::uint64_t size =
        read_binary<std::uint64_t>(data, pos);

    if (size > data.size() - pos) {
        throw std::runtime_error(
            "transaction string truncated"
        );
    }

    std::string value =
        data.substr(
            pos,
            static_cast<std::size_t>(size)
        );

    pos += static_cast<std::size_t>(size);

    return value;
}

} // namespace

namespace larb {

Transaction Transaction::deserialize(
    const std::string& data
) {
    std::size_t pos = 0;

    Transaction tx{};

    tx.version =
        read_binary<std::uint32_t>(data, pos);

    const std::uint64_t input_count =
        read_binary<std::uint64_t>(data, pos);

    for (std::uint64_t i = 0;
         i < input_count;
         ++i) {

        TransactionInput input;

        input.previous_txid =
            read_binary_string(data, pos);

        input.output_index =
            read_binary<std::uint32_t>(data, pos);

        input.public_key =
            read_binary_string(data, pos);

        input.signature =
            read_binary_string(data, pos);

        tx.inputs.push_back(
            std::move(input)
        );
    }

    const std::uint64_t output_count =
        read_binary<std::uint64_t>(data, pos);

    for (std::uint64_t i = 0;
         i < output_count;
         ++i) {

        TransactionOutput output;

        output.amount =
            read_binary<std::int64_t>(data, pos);

        output.script_pubkey =
            read_binary_string(data, pos);

        tx.outputs.push_back(
            std::move(output)
        );
    }

    if (pos != data.size()) {
        const std::string marker =
            read_binary_string(data, pos);

        if (marker != "LARB-CB1") {
            throw std::runtime_error(
                "unknown transaction metadata"
            );
        }

        tx.coinbase_data =
            read_binary_string(data, pos);

        if (tx.coinbase_data.empty()) {
            throw std::runtime_error(
                "empty coinbase metadata"
            );
        }

        if (pos != data.size()) {
            throw std::runtime_error(
                "extra transaction data"
            );
        }
    }

    return tx;
}

} // namespace larb
