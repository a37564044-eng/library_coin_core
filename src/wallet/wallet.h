#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace larb {

class Wallet {
public:
    Wallet();
    ~Wallet();

    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) noexcept = default;
    Wallet& operator=(Wallet&&) noexcept = default;
    Wallet& operator=(const Wallet&) = delete;

    const std::vector<uint8_t>& public_key() const;
    const std::vector<uint8_t>& secret_key() const;
    std::string address() const;

    std::string sign(const std::string& message) const;

    bool verify(
        const std::string& message,
        const std::string& signature
    ) const;

    bool save(const std::string& path, const std::string& password) const;

    static Wallet load(const std::string& path, const std::string& password);

    static constexpr std::size_t PUBLIC_KEY_SIZE = 1312;
    static constexpr std::size_t SECRET_KEY_SIZE = 2560;
    static constexpr std::size_t SIGNATURE_SIZE = 2420;

private:
    Wallet(
        std::vector<uint8_t> public_key,
        std::vector<uint8_t> secret_key
    );

    std::vector<uint8_t> public_key_;
    std::vector<uint8_t> secret_key_;
};

} // namespace larb
