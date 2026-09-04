#include "persistence.h"

#include "chain_codec.h"
#include "consensus/constants.h"

#include <openssl/sha.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace larb {

namespace {

constexpr const char* MAGIC = "LARB_STATE_V1";
constexpr std::uint32_t FORMAT_VERSION = 1;

std::string hex_encode(const std::string& input) {
    std::ostringstream out;

    for (unsigned char c : input) {
        out << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<unsigned int>(c);
    }

    return out.str();
}

bool hex_value(char c, std::uint8_t& value) {
    if (c >= '0' && c <= '9') {
        value = static_cast<std::uint8_t>(c - '0');
        return true;
    }

    if (c >= 'a' && c <= 'f') {
        value = static_cast<std::uint8_t>(c - 'a' + 10);
        return true;
    }

    if (c >= 'A' && c <= 'F') {
        value = static_cast<std::uint8_t>(c - 'A' + 10);
        return true;
    }

    return false;
}

bool hex_decode(
    const std::string& input,
    std::string& output
) {
    if (input.size() % 2 != 0) {
        return false;
    }

    output.clear();
    output.reserve(input.size() / 2);

    for (std::size_t i = 0; i < input.size(); i += 2) {
        std::uint8_t hi = 0;
        std::uint8_t lo = 0;

        if (!hex_value(input[i], hi) ||
            !hex_value(input[i + 1], lo)) {
            return false;
        }

        output.push_back(
            static_cast<char>(
                static_cast<std::uint8_t>(
                    (hi << 4) | lo
                )
            )
        );
    }

    return true;
}

std::string sha256_hex(
    const std::string& data
) {
    unsigned char digest[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(
            data.data()
        ),
        data.size(),
        digest
    );

    std::ostringstream out;

    for (unsigned char byte : digest) {
        out << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<unsigned int>(byte);
    }

    return out.str();
}

std::string build_payload(
    const Blockchain& chain,
    const UTXOSet& utxos
) {
    std::ostringstream out;

    out << MAGIC << '\n';
    out << FORMAT_VERSION << '\n';

    /*
     * Chain adalah consensus state.
     * serialize_chain() menyimpan difficulty,
     * block count, seluruh block dan transaksi.
     */
    const std::string chain_data =
        serialize_chain(chain);

    out << chain_data.size() << '\n';
    out << chain_data;

    /*
     * Metadata chain.
     */
    out << "METADATA\n";
    out << chain.size() << '\n';
    out << chain.difficulty() << '\n';
    out << chain.at(chain.size() - 1).hash() << '\n';

    /*
     * Deterministic UTXO snapshot.
     */
    const std::vector<UTXO> snapshot =
        utxos.snapshot();

    out << "UTXO\n";
    out << snapshot.size() << '\n';

    for (const auto& item : snapshot) {
        out << hex_encode(item.outpoint.txid) << '\n';
        out << item.outpoint.output_index << '\n';
        out << item.output.amount << '\n';
        out << hex_encode(item.output.script_pubkey) << '\n';
    }

    return out.str();
}

bool read_line(
    const std::string& data,
    std::size_t& pos,
    std::string& line
) {
    const std::size_t end =
        data.find('\n', pos);

    if (end == std::string::npos) {
        return false;
    }

    line = data.substr(pos, end - pos);
    pos = end + 1;

    return true;
}

bool parse_size(
    const std::string& text,
    std::size_t& value
) {
    if (text.empty()) {
        return false;
    }

    try {
        std::size_t used = 0;

        const unsigned long long parsed =
            std::stoull(text, &used);

        if (used != text.size()) {
            return false;
        }

        value = static_cast<std::size_t>(parsed);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool parse_u32(
    const std::string& text,
    std::uint32_t& value
) {
    try {
        std::size_t used = 0;

        const unsigned long long parsed =
            std::stoull(text, &used);

        if (used != text.size() ||
            parsed > UINT32_MAX) {
            return false;
        }

        value =
            static_cast<std::uint32_t>(parsed);

        return true;
    }
    catch (...) {
        return false;
    }
}

bool parse_i64(
    const std::string& text,
    std::int64_t& value
) {
    try {
        std::size_t used = 0;

        const long long parsed =
            std::stoll(text, &used);

        if (used != text.size()) {
            return false;
        }

        value =
            static_cast<std::int64_t>(parsed);

        return true;
    }
    catch (...) {
        return false;
    }
}

bool atomic_write(
    const std::string& path,
    const std::string& data
) {
    const std::string tmp =
        path + ".tmp";

    const int fd =
        ::open(
            tmp.c_str(),
            O_WRONLY | O_CREAT | O_TRUNC,
            0600
        );

    if (fd < 0) {
        return false;
    }

    const char* ptr = data.data();
    std::size_t remaining = data.size();

    while (remaining > 0) {
        const ssize_t written =
            ::write(
                fd,
                ptr,
                remaining
            );

        if (written <= 0) {
            ::close(fd);
            ::unlink(tmp.c_str());
            return false;
        }

        ptr += written;
        remaining -=
            static_cast<std::size_t>(written);
    }

    if (::fsync(fd) != 0) {
        ::close(fd);
        ::unlink(tmp.c_str());
        return false;
    }

    if (::close(fd) != 0) {
        ::unlink(tmp.c_str());
        return false;
    }

    if (::rename(
            tmp.c_str(),
            path.c_str()) != 0) {
        ::unlink(tmp.c_str());
        return false;
    }

    return true;
}

} // namespace

bool Persistence::save(
    const std::string& path,
    const Blockchain& chain,
    const UTXOSet& utxos
) {
    if (chain.size() == 0) {
        return false;
    }

    if (!chain.is_valid()) {
        return false;
    }

    const std::string payload =
        build_payload(chain, utxos);

    const std::string checksum =
        sha256_hex(payload);

    std::ostringstream file;

    file << payload;
    file << "CHECKSUM\n";
    file << checksum << '\n';

    return atomic_write(
        path,
        file.str()
    );
}

bool Persistence::load(
    const std::string& path,
    Blockchain& chain,
    UTXOSet& utxos
) {
    std::ifstream input(
        path,
        std::ios::binary
    );

    if (!input) {
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    const std::string file =
        buffer.str();

    const std::size_t checksum_pos =
        file.rfind("CHECKSUM\n");

    if (checksum_pos == std::string::npos) {
        return false;
    }

    const std::string payload =
        file.substr(0, checksum_pos);

    const std::string checksum_block =
        file.substr(
            checksum_pos +
            std::string("CHECKSUM\n").size()
        );

    if (checksum_block.size() != 65 ||
        checksum_block.back() != '\n') {
        return false;
    }

    const std::string expected =
        checksum_block.substr(
            0,
            checksum_block.size() - 1
        );

    if (sha256_hex(payload) != expected) {
        return false;
    }

    try {
        std::size_t pos = 0;
        std::string line;

        if (!read_line(payload, pos, line) ||
            line != MAGIC) {
            return false;
        }

        if (!read_line(payload, pos, line) ||
            line != std::to_string(FORMAT_VERSION)) {
            return false;
        }

        if (!read_line(payload, pos, line)) {
            return false;
        }

        std::size_t chain_size = 0;

        if (!parse_size(line, chain_size) ||
            chain_size == 0 ||
            chain_size > payload.size() - pos) {
            return false;
        }

        const std::string chain_data =
            payload.substr(pos, chain_size);

        pos += chain_size;

        const auto decoded =
            deserialize_chain(chain_data);

        if (!decoded) {
            return false;
        }

        if (!read_line(payload, pos, line) ||
            line != "METADATA") {
            return false;
        }

        std::size_t metadata_height = 0;

        if (!read_line(payload, pos, line) ||
            !parse_size(line, metadata_height)) {
            return false;
        }

        std::uint32_t metadata_difficulty = 0;

        if (!read_line(payload, pos, line) ||
            !parse_u32(line, metadata_difficulty)) {
            return false;
        }

        std::string metadata_tip;

        if (!read_line(payload, pos, metadata_tip)) {
            return false;
        }

        if (metadata_height != decoded->size() ||
            metadata_difficulty != decoded->difficulty() ||
            metadata_tip !=
                decoded->at(
                    decoded->size() - 1
                ).hash()) {
            return false;
        }

        if (!read_line(payload, pos, line) ||
            line != "UTXO") {
            return false;
        }

        std::size_t utxo_count = 0;

        if (!read_line(payload, pos, line) ||
            !parse_size(line, utxo_count)) {
            return false;
        }

        UTXOSet loaded_utxos;

        for (std::size_t i = 0;
             i < utxo_count;
             ++i) {

            std::string txid_hex;

            if (!read_line(
                    payload,
                    pos,
                    txid_hex)) {
                return false;
            }

            std::string txid;

            if (!hex_decode(
                    txid_hex,
                    txid)) {
                return false;
            }

            std::uint32_t index = 0;

            if (!read_line(payload, pos, line) ||
                !parse_u32(line, index)) {
                return false;
            }

            std::int64_t amount = 0;

            if (!read_line(payload, pos, line) ||
                !parse_i64(line, amount)) {
                return false;
            }

            std::string script_hex;

            if (!read_line(
                    payload,
                    pos,
                    script_hex)) {
                return false;
            }

            std::string script;

            if (!hex_decode(
                    script_hex,
                    script)) {
                return false;
            }

            if (amount < 0 ||
                amount > MAX_MONEY) {
                return false;
            }

            loaded_utxos.add(
                UTXO{
                    OutPoint{
                        txid,
                        index
                    },
                    TransactionOutput{
                        amount,
                        script
                    }
                }
            );
        }

        if (pos != payload.size()) {
            return false;
        }

        chain = *decoded;
        utxos.replace_with(loaded_utxos);

        return true;
    }
    catch (...) {
        return false;
    }
}

} // namespace larb
