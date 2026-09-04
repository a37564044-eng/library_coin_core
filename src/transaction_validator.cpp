#include "transaction_validator.h"
#include "consensus/constants.h"
#include "script.h"
#include "address_codec.h"
#include <openssl/sha.h>
#include <string>

#include <unordered_set>
#include <functional>
#include <cstdint>
#include <limits>

namespace larb {

struct LocalOutPointHash {
    std::size_t operator()(const OutPoint& point) const {
        const std::size_t h1 = std::hash<std::string>{}(point.txid);
        const std::size_t h2 = std::hash<std::uint32_t>{}(point.output_index);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

bool validate_transaction_inputs(
    const Transaction& tx,
    const UTXOSet& utxos
) {
    std::unordered_set<OutPoint, LocalOutPointHash> seen;

    for (std::size_t input_index = 0;
         input_index < tx.inputs.size();
         ++input_index) {

        const auto& input = tx.inputs[input_index];

        if (input.public_key.empty() ||
            input.signature.empty()) {
            return false;
        }

        OutPoint outpoint{
            input.previous_txid,
            input.output_index
        };

        if (!seen.insert(outpoint).second) {
            return false;
        }

        const UTXO* utxo = utxos.find(outpoint);
        if (utxo == nullptr) {
            return false;
        }

        if (!Script::is_pay_to_pubkey_hash(
                utxo->output.script_pubkey)) {
            return false;
        }

        unsigned char digest[SHA256_DIGEST_LENGTH];

        SHA256(
            reinterpret_cast<const unsigned char*>(input.public_key.data()),
            input.public_key.size(),
            digest
        );

        const std::string expected_hash(
            reinterpret_cast<const char*>(digest),
            SHA256_DIGEST_LENGTH
        );

        const std::string actual_hash =
            Script::extract_pubkey_hash(
                utxo->output.script_pubkey
            );

        if (expected_hash != actual_hash) {
            return false;
        }

        if (!tx.verify_input(input_index)) {
            return false;
        }
    }

    return true;
}

bool validate_transaction_value(
    const Transaction& tx,
    const UTXOSet& utxos
) {
    if (tx.inputs.empty() || tx.outputs.empty()) {
        return false;
    }

    std::int64_t input_value = 0;
    std::vector<OutPoint> seen_inputs;

    for (const auto& input : tx.inputs) {
        OutPoint outpoint{
            input.previous_txid,
            input.output_index
        };

        for (const auto& seen : seen_inputs) {
            if (seen == outpoint) {
                return false;
            }
        }

        seen_inputs.push_back(outpoint);

        const UTXO* utxo = utxos.find(outpoint);
        if (utxo == nullptr) {
            return false;
        }

        if (utxo->output.amount < 0 ||
            utxo->output.amount > MAX_MONEY) {
            return false;
        }

        if (input_value > INT64_MAX - utxo->output.amount) {
            return false;
        }

        input_value += utxo->output.amount;

        if (input_value > MAX_MONEY) {
            return false;
        }
    }

    std::int64_t output_value = 0;

    for (const auto& output : tx.outputs) {
        if (output.amount < 0 ||
            output.amount > MAX_MONEY) {
            return false;
        }

        if (output_value > INT64_MAX - output.amount) {
            return false;
        }

        output_value += output.amount;

        if (output_value > MAX_MONEY) {
            return false;
        }
    }

    return output_value <= input_value;
}

bool validate_coinbase(
    const Transaction& tx,
    std::int64_t block_reward
) {
    if (block_reward <= 0) {
        return false;
    }

    if (block_reward > MAX_MONEY) {
        return false;
    }

    if (!tx.inputs.empty()) {
        return false;
    }

    if (tx.outputs.empty()) {
        return false;
    }

    std::int64_t total = 0;

    for (const auto& output : tx.outputs) {
        if (output.amount < 0) {
            return false;
        }

        if (output.amount > MAX_MONEY) {
            return false;
        }

        if (!AddressCodec::is_valid(output.script_pubkey)) {
            return false;
        }

        if (total >
            INT64_MAX - output.amount) {
            return false;
        }

        total += output.amount;

        if (total > MAX_MONEY) {
            return false;
        }
    }

    return total == block_reward;
}

bool apply_coinbase(
    const Transaction& tx,
    std::int64_t block_reward,
    UTXOSet& utxos
) {
    if (!validate_coinbase(tx, block_reward)) {
        return false;
    }

    const std::string txid = tx.txid();

    for (std::size_t i = 0;
         i < tx.outputs.size();
         ++i) {

        UTXO utxo{
            OutPoint{
                txid,
                static_cast<std::uint32_t>(i)
            },
            tx.outputs[i]
        };

        utxos.add(utxo);
    }

    return true;
}

bool apply_transaction(
    const Transaction& tx,
    UTXOSet& utxos
) {
    if (!validate_transaction_value(
            tx,
            utxos)) {
        return false;
    }

    for (const auto& input : tx.inputs) {
        if (!input.public_key.empty() || !input.signature.empty()) {
            const std::size_t input_index =
                static_cast<std::size_t>(&input - tx.inputs.data());
            if (!tx.verify_input(input_index)) {
                return false;
            }
        }
        OutPoint outpoint{
            input.previous_txid,
            input.output_index
        };

        if (!utxos.spend(outpoint)) {
            return false;
        }
    }

    const std::string txid = tx.txid();

    for (std::size_t i = 0;
         i < tx.outputs.size();
         ++i) {

        UTXO utxo{
            OutPoint{
                txid,
                static_cast<std::uint32_t>(i)
            },
            tx.outputs[i]
        };

        utxos.add(utxo);
    }

    return true;
}


bool calculate_transaction_fee(
    const Transaction& tx,
    const UTXOSet& utxos,
    std::int64_t& fee
) {
    fee = 0;

    if (!validate_transaction_inputs(tx, utxos)) {
        return false;
    }

    std::int64_t input_value = 0;
    std::int64_t output_value = 0;

    for (const auto& input : tx.inputs) {
        if (!input.public_key.empty() || !input.signature.empty()) {
            const std::size_t input_index =
                static_cast<std::size_t>(&input - tx.inputs.data());
            if (!tx.verify_input(input_index)) {
                return false;
            }
        }
        OutPoint outpoint{
            input.previous_txid,
            input.output_index
        };

        const UTXO* utxo = utxos.find(outpoint);

        if (utxo == nullptr) {
            return false;
        }

        if (utxo->output.amount < 0 ||
            utxo->output.amount > MAX_MONEY) {
            return false;
        }

        if (input_value >
            INT64_MAX - utxo->output.amount) {
            return false;
        }

        input_value += utxo->output.amount;
    }

    for (const auto& output : tx.outputs) {
        if (output.amount < 0 ||
            output.amount > MAX_MONEY) {
            return false;
        }

        if (output_value >
            INT64_MAX - output.amount) {
            return false;
        }

        output_value += output.amount;

        if (output_value > MAX_MONEY) {
            return false;
        }
    }

    if (output_value > input_value) {
        return false;
    }

    fee = input_value - output_value;

    return true;
}

} // namespace larb
