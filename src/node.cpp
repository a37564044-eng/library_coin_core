#include "node.h"

#include "block_validator.h"
#include "persistence.h"
#include "consensus/pow.h"

namespace larb {

Node::Node(
    const Block& genesis,
    std::uint32_t difficulty
)
    : blockchain_(genesis, difficulty),
      utxos_{},
      difficulty_(difficulty) {
}

bool Node::receive_block(const Block& block) {
    if (blockchain_.size() == 0) {
        return false;
    }

    if (block.header().previous_hash !=
        blockchain_.at(blockchain_.size() - 1).hash()) {
        return false;
    }

    if (!validate_proof_of_work(
            block,
            difficulty_)) {
        return false;
    }

    if (!validate_block(
            block,
            blockchain_.size(),
            utxos_)) {
        return false;
    }

    return blockchain_.add_block(block);
}

bool Node::adopt_chain(
    const Blockchain& candidate
) {
    /*
     * Chain kosong tidak boleh diadopsi.
     */
    if (candidate.size() == 0) {
        return false;
    }

    /*
     * Jangan mengganti chain dengan chain
     * yang sama panjang atau lebih pendek.
     */
    if (candidate.chain_work() <= blockchain_.chain_work()) {
        return false;
    }

    /*
     * Validasi struktur + PoW seluruh candidate.
     */
    if (!candidate.is_valid()) {
        return false;
    }

    /*
     * Bangun UTXO dari nol.
     *
     * Jangan menyentuh state node terlebih dahulu.
     * Kalau ada satu block gagal, seluruh proses
     * dibatalkan.
     */
    UTXOSet rebuilt_utxos;

    for (std::size_t height = 1;
         height < candidate.size();
         ++height) {

        const Block& block =
            candidate.at(height);

        if (!validate_block(
                block,
                height,
                rebuilt_utxos)) {
            return false;
        }
    }

    /*
     * Semua validasi berhasil.
     * Commit chain + UTXO secara bersamaan.
     */
    blockchain_ = candidate;
    utxos_.replace_with(rebuilt_utxos);

    return true;
}

bool Node::is_chain_valid() const {
    return blockchain_.is_valid();
}

std::size_t Node::chain_size() const {
    return blockchain_.size();
}

const Blockchain& Node::blockchain() const {
    return blockchain_;
}

const UTXOSet& Node::utxos() const {
    return utxos_;
}

} // namespace larb

bool larb::Node::load_state(const std::string& path) {
    return larb::Persistence::load(path, blockchain_, utxos_);
}

bool larb::Node::save_state(const std::string& path) const {
    return larb::Persistence::save(path, blockchain_, utxos_);
}
