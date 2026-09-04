#include "genesis.h"
#include "node.h"
#include "persistence.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    const std::string good = "persistence_good.dat";
    const std::string bad  = "persistence_corrupt.dat";

    const larb::Block genesis = larb::Genesis::create();
    larb::Node node(genesis, 3);

    // Simpan state genesis yang valid.
    assert(node.save_state(good));

    // Buat salinan lalu korupsi byte pertama.
    {
        std::ifstream in(good, std::ios::binary);
        std::ofstream out(bad, std::ios::binary);

        assert(in.good());
        assert(out.good());

        std::string data(
            (std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>()
        );

        assert(!data.empty());
        data[0] ^= static_cast<char>(0xff);

        out.write(data.data(),
                  static_cast<std::streamsize>(data.size()));
        assert(out.good());
    }

    // State korup wajib ditolak.
    larb::Node recovered(genesis, 3);
    assert(!recovered.load_state(bad));

    // State asli tetap dapat dimuat.
    larb::Node valid(genesis, 3);
    assert(valid.load_state(good));
    assert(valid.chain_size() == 1);
    assert(valid.is_chain_valid());

    std::remove(good.c_str());
    std::remove(bad.c_str());

    std::cout << "Persistence corruption test: PASS\n";
    return 0;
}
