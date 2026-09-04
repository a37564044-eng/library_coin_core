#include "crypto/pqc.h"

namespace larb {

std::string PQC::sign(
    const std::string&,
    const std::string&
) {
    return {};
}

bool PQC::verify(
    const std::string&,
    const std::string&,
    const std::string&
) {
    return false;
}

PQCKeyPair PQC::generate_keypair() {
    return {};
}

std::string PQC::sign_hex(
    const std::string&,
    const std::string&
) {
    return {};
}

bool PQC::verify_hex(
    const std::string&,
    const std::string&,
    const std::string&
) {
    return false;
}

} // namespace larb
