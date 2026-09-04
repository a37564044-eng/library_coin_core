#include "message.h"

#include "script.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace larb {

namespace {

constexpr const char* MAGIC = "LARBMSG1";
constexpr std::size_t HEADER_LIMIT = 50;

std::string make_header(
    const std::string& document_id,
    std::uint32_t index,
    std::uint32_t total
) {
    std::ostringstream out;

    out << MAGIC
        << "|"
        << document_id
        << "|"
        << index
        << "|"
        << total
        << "|";

    return out.str();
}

} // namespace

std::vector<std::string> Message::encode(
    const std::string& document_id,
    const std::string& message
) {
    if (document_id.empty()) {
        throw std::runtime_error(
            "document id cannot be empty"
        );
    }

    if (document_id.find('|') != std::string::npos) {
        throw std::runtime_error(
            "document id contains reserved character"
        );
    }

    /*
     * Cari ukuran chunk berdasarkan header aktual.
     * Header setiap chunk berbeda karena index/total.
     */
    std::size_t total = 1;

    while (true) {
        const std::size_t header_size =
            make_header(
                document_id,
                static_cast<std::uint32_t>(total - 1),
                static_cast<std::uint32_t>(total)
            ).size();

        if (header_size >= OP_RETURN_LIMIT) {
            throw std::runtime_error(
                "message metadata exceeds OP_RETURN limit"
            );
        }

        const std::size_t payload_size =
            OP_RETURN_LIMIT - header_size;

        const std::size_t calculated =
            message.empty()
                ? 1
                : (message.size() + payload_size - 1)
                    / payload_size;

        if (calculated == total) {
            break;
        }

        total = calculated;
    }

    std::vector<std::string> result;

    std::size_t offset = 0;

    for (std::size_t index = 0;
         index < total;
         ++index) {

        const std::string header =
            make_header(
                document_id,
                static_cast<std::uint32_t>(index),
                static_cast<std::uint32_t>(total)
            );

        if (header.size() >= OP_RETURN_LIMIT) {
            throw std::runtime_error(
                "message header too large"
            );
        }

        const std::size_t capacity =
            OP_RETURN_LIMIT - header.size();

        const std::size_t remaining =
            message.size() - offset;

        const std::size_t count =
            std::min(capacity, remaining);

        std::string payload =
            header +
            message.substr(offset, count);

        /*
         * Pastikan payload yang diberikan ke Script
         * tidak pernah melewati 80 byte.
         */
        if (payload.size() > OP_RETURN_LIMIT) {
            throw std::runtime_error(
                "encoded message chunk exceeds 80 bytes"
            );
        }

        result.push_back(
            Script::op_return(payload)
        );

        offset += count;
    }

    return result;
}

MessageChunk Message::decode(
    const std::string& payload
) {
    const std::string data =
        Script::extract_op_return_data(payload);

    std::stringstream stream(data);

    std::string magic;
    std::string document_id;
    std::string index_string;
    std::string total_string;
    std::string message;

    if (!std::getline(stream, magic, '|') ||
        !std::getline(stream, document_id, '|') ||
        !std::getline(stream, index_string, '|') ||
        !std::getline(stream, total_string, '|')) {
        throw std::runtime_error(
            "invalid LARB message"
        );
    }

    if (magic != MAGIC) {
        throw std::runtime_error(
            "not a LARB message"
        );
    }

    std::getline(stream, message);

    std::uint32_t index = 0;
    std::uint32_t total = 0;

    try {
        index =
            static_cast<std::uint32_t>(
                std::stoul(index_string)
            );

        total =
            static_cast<std::uint32_t>(
                std::stoul(total_string)
            );
    } catch (...) {
        throw std::runtime_error(
            "invalid message index"
        );
    }

    if (total == 0 || index >= total) {
        throw std::runtime_error(
            "invalid message ordering"
        );
    }

    return MessageChunk{
        document_id,
        index,
        total,
        message
    };
}

std::string Message::assemble(
    std::vector<MessageChunk> chunks
) {
    if (chunks.empty()) {
        return "";
    }

    const std::string& document_id =
        chunks.front().document_id;

    const std::uint32_t total =
        chunks.front().total;

    if (chunks.size() != total) {
        throw std::runtime_error(
            "incomplete message"
        );
    }

    std::sort(
        chunks.begin(),
        chunks.end(),
        [](const MessageChunk& a,
           const MessageChunk& b) {
            return a.index < b.index;
        }
    );

    std::string result;

    for (std::uint32_t i = 0; i < total; ++i) {
        const auto& chunk = chunks[i];

        if (chunk.document_id != document_id ||
            chunk.total != total ||
            chunk.index != i) {
            throw std::runtime_error(
                "message chunks are inconsistent"
            );
        }

        result += chunk.data;
    }

    return result;
}

} // namespace larb
