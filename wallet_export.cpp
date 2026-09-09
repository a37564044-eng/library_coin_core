




#include "src/wallet/wallet.h"
#include <iomanip>
#include <iostream>
#include <string>

int main() {
    try {
        std::string password;

        std::cout << "Wallet password: ";
        std::getline(std::cin, password);

        larb::Wallet wallet =
            larb::Wallet::load("/public/.larb/wallet.dat", password);

        const auto& key = wallet.secret_key();

        std::cout << "\nWallet load: OK\n";
        std::cout << "Address: " << wallet.address() << "\n";
        std::cout << "Private key size: " << key.size() << " bytes\n";
        std::cout << "Private key HEX:\n";

        for (std::uint8_t byte : key) {
            std::cout << std::hex
                      << std::setw(2)
                      << std::setfill('0')
                      << static_cast<unsigned int>(byte);
        }

        std::cout << "\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Wallet export FAILED: "
                  << e.what() << "\n";
        return 1;
    }
}
