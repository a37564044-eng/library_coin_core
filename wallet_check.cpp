#include "src/wallet/wallet.h"
#include <iostream>
#include <string>

int main() {
    try {
        std::string password;

        std::cout << "Wallet password: ";
        std::getline(std::cin, password);

        larb::Wallet wallet =
            larb::Wallet::load("/public/.larb/wallet.dat", password);

        std::cout << "\nWallet load: OK\n";
        std::cout << "Address: " << wallet.address() << "\n";

        const std::string test_message =
            "LARB-WALLET-OWNERSHIP-CHECK";

        const std::string signature =
            wallet.sign(test_message);

        std::cout << "Signature: OK\n";
        std::cout << "Signature verification: "
                  << (wallet.verify(test_message, signature) ? "OK" : "FAILED")
                  << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Wallet check FAILED: "
                  << e.what() << "\n";
        return 1;
    }

    return 0;
}
