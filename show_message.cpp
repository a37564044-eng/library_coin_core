#include "src/blockchain.h"
#include "src/genesis.h"
#include "src/persistence.h"
#include "src/utxo_set.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

int main() {
    larb::Blockchain chain(larb::Genesis::create(), 3);
    larb::UTXOSet utxos;

    if (!larb::Persistence::load("larb_chain.dat", chain, utxos)) {
        std::cerr << "Gagal memuat blockchain.\n";
        return 1;
    }

    std::vector<std::string> chunks(6);
    bool found[6] = {false, false, false, false, false, false};

    const std::string marker = "LARBMSG1|BLOCK2-KHARISA|";

    for (std::size_t i = 0; i < chain.size(); ++i) {
        for (const auto& tx : chain.at(i).transactions()) {
            std::size_t pos = 0;

            while ((pos = tx.find(marker, pos)) != std::string::npos) {
                const std::size_t index_start = pos + marker.size();
                const std::size_t separator = tx.find('|', index_start);

                if (separator == std::string::npos)
                    break;

                try {
                    const int index =
                        std::stoi(tx.substr(index_start, separator - index_start));

                    if (index >= 0 && index < 6) {
                        const std::size_t text_start = separator + 1;
                        const std::size_t next_marker =
                            tx.find(marker, text_start);

                        chunks[index] =
                            tx.substr(
                                text_start,
                                next_marker == std::string::npos
                                    ? std::string::npos
                                    : next_marker - text_start
                            );

                        found[index] = true;
                    }
                } catch (...) {
                }

                pos = separator + 1;
            }
        }
    }

    std::cout << "\n========== LARB BLOCK #2 MESSAGE ==========\n\n";

    bool complete = true;

    for (int i = 0; i < 6; ++i) {
        if (!found[i]) {
            complete = false;
            std::cout << "[CHUNK " << i << "] MISSING\n";
        }
    }

    if (!complete) {
        std::cout << "\nPesan belum lengkap.\n";
        return 1;
    }

    std::cout << "Pesan lengkap:\n\n";

    for (int i = 0; i < 6; ++i)
        std::cout << chunks[i];

    std::cout << "\n\n============================================\n";
    std::cout << "6/6 chunk ditemukan.\n";
    std::cout << "Blockchain: READ-ONLY\n";

    return 0;
}
