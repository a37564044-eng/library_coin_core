#include "src/wallet/wallet.h"
#include <iostream>
#include <string>

static const std::string BLOCK1_ADDRESS =
    "larb1q5uz4wm4fannwe7c5ql4ypzkg2ygdszessfnx9ef7mka5mufy08wslnjkpr";

int main() {
    try {
        std::string password;

        std::cout << "Wallet password: ";
        std::getline(std::cin, password);

        const char* paths[] = {
            "/public/.larb/wallet.dat",
            "/public/library_coin_core/larb_wallet.dat"
        };

        for (const char* path : paths) {
            std::cout << "\n=== " << path << " ===\n";

            try {
                larb::Wallet wallet =
                    larb::Wallet::load(path, password);

                const std::string address = wallet.address();

                std::cout << "Load      : OK\n";
                std::cout << "Address   : " << address << "\n";
                std::cout << "Block 1   : "
                          << (address == BLOCK1_ADDRESS ? "MATCH" : "NO MATCH")
                          << "\n";
            }
            catch (const std::exception& e) {
                std::cout << "Load      : FAILED\n";
                std::cout << "Reason    : " << e.what() << "\n";
            }
        }

        std::cout << "\n=== RESULT ===\n";
        std::cout << "Target Block 1 address:\n"
                  << BLOCK1_ADDRESS << "\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}
