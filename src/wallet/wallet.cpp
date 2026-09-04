#include "wallet.h"

#include <oqs/oqs.h>
#include <oqs/sig_ml_dsa.h>
#include <openssl/sha.h>

#include "address_codec.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace larb {

namespace {

constexpr char WALLET_MAGIC[] = "LARBWLT1";
constexpr std::size_t WALLET_MAGIC_SIZE =
    sizeof(WALLET_MAGIC) - 1;

}

Wallet::Wallet()
    : public_key_(PUBLIC_KEY_SIZE),
      secret_key_(SECRET_KEY_SIZE) {

    if (OQS_SIG_ml_dsa_44_keypair(
            public_key_.data(),
            secret_key_.data()) != OQS_SUCCESS) {
        throw std::runtime_error(
            "ML-DSA-44 keypair generation failed"
        );
    }
}

Wallet::Wallet(
    std::vector<uint8_t> public_key,
    std::vector<uint8_t> secret_key
)
    : public_key_(std::move(public_key)),
      secret_key_(std::move(secret_key)) {

    if (public_key_.size() != PUBLIC_KEY_SIZE ||
        secret_key_.size() != SECRET_KEY_SIZE) {
        throw std::runtime_error(
            "invalid wallet key size"
        );
    }
}

Wallet::~Wallet() {
    std::fill(
        secret_key_.begin(),
        secret_key_.end(),
        static_cast<uint8_t>(0)
    );

    std::fill(
        public_key_.begin(),
        public_key_.end(),
        static_cast<uint8_t>(0)
    );
}

const std::vector<uint8_t>& Wallet::public_key() const {
    return public_key_;
}

const std::vector<uint8_t>& Wallet::secret_key() const {
    return secret_key_;
}

std::string Wallet::address() const {
    unsigned char digest[SHA256_DIGEST_LENGTH];

    SHA256(
        public_key_.data(),
        public_key_.size(),
        digest
    );

    std::vector<std::uint8_t> payload(
        digest,
        digest + SHA256_DIGEST_LENGTH
    );

    return AddressCodec::encode(payload);
}

std::string Wallet::sign(
    const std::string& message
) const {
    std::vector<uint8_t> signature(SIGNATURE_SIZE);
    size_t signature_len = 0;

    if (OQS_SIG_ml_dsa_44_sign(
            signature.data(),
            &signature_len,
            reinterpret_cast<const uint8_t*>(
                message.data()
            ),
            message.size(),
            secret_key_.data()) != OQS_SUCCESS) {
        throw std::runtime_error(
            "ML-DSA-44 signing failed"
        );
    }

    return std::string(
        reinterpret_cast<const char*>(
            signature.data()
        ),
        signature_len
    );
}

bool Wallet::verify(
    const std::string& message,
    const std::string& signature
) const {
    if (signature.size() != SIGNATURE_SIZE)
        return false;

    return OQS_SIG_ml_dsa_44_verify(
        reinterpret_cast<const uint8_t*>(
            message.data()
        ),
        message.size(),
        reinterpret_cast<const uint8_t*>(
            signature.data()
        ),
        signature.size(),
        public_key_.data()
    ) == OQS_SUCCESS;
}

bool Wallet::save(
    const std::string& path,
    const std::string& password
) const {
    if (password.empty())
        return false;

    constexpr char MAGIC[] = "LARBWLT2";
    constexpr std::size_t SALT_SIZE = 16;
    constexpr std::size_t NONCE_SIZE = 12;
    constexpr std::size_t TAG_SIZE = 16;
    constexpr int ITERATIONS = 200000;

    std::array<unsigned char, SALT_SIZE> salt{};
    std::array<unsigned char, NONCE_SIZE> nonce{};
    std::array<unsigned char, 32> key{};

    if (RAND_bytes(salt.data(), salt.size()) != 1 ||
        RAND_bytes(nonce.data(), nonce.size()) != 1) {
        return false;
    }

    if (PKCS5_PBKDF2_HMAC(
            password.data(),
            static_cast<int>(password.size()),
            salt.data(),
            static_cast<int>(salt.size()),
            ITERATIONS,
            EVP_sha256(),
            static_cast<int>(key.size()),
            key.data()) != 1) {
        return false;
    }

    std::vector<unsigned char> ciphertext(
        secret_key_.size()
    );

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return false;

    int len = 0;
    int ciphertext_len = 0;
    bool ok = true;

    if (EVP_EncryptInit_ex(
            ctx,
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr) != 1)
        ok = false;

    if (ok && EVP_CIPHER_CTX_ctrl(
            ctx,
            EVP_CTRL_GCM_SET_IVLEN,
            NONCE_SIZE,
            nullptr) != 1)
        ok = false;

    if (ok && EVP_EncryptInit_ex(
            ctx,
            nullptr,
            nullptr,
            key.data(),
            nonce.data()) != 1)
        ok = false;

    if (ok && EVP_EncryptUpdate(
            ctx,
            nullptr,
            &len,
            reinterpret_cast<const unsigned char*>(MAGIC),
            sizeof(MAGIC) - 1) != 1)
        ok = false;

    if (ok && EVP_EncryptUpdate(
            ctx,
            nullptr,
            &len,
            public_key_.data(),
            static_cast<int>(public_key_.size())) != 1)
        ok = false;

    if (ok && EVP_EncryptUpdate(
            ctx,
            ciphertext.data(),
            &len,
            secret_key_.data(),
            static_cast<int>(secret_key_.size())) != 1)
        ok = false;

    ciphertext_len = len;

    if (ok && EVP_EncryptFinal_ex(
            ctx,
            ciphertext.data() + ciphertext_len,
            &len) != 1)
        ok = false;

    ciphertext_len += len;

    std::array<unsigned char, TAG_SIZE> tag{};

    if (ok && EVP_CIPHER_CTX_ctrl(
            ctx,
            EVP_CTRL_GCM_GET_TAG,
            TAG_SIZE,
            tag.data()) != 1)
        ok = false;

    EVP_CIPHER_CTX_free(ctx);

    std::fill(key.begin(), key.end(), 0);

    if (!ok)
        return false;

    std::string temporary = path + ".tmp";

    std::ofstream file(
        temporary,
        std::ios::binary | std::ios::trunc
    );

    if (!file)
        return false;

    const std::uint32_t public_size =
        static_cast<std::uint32_t>(public_key_.size());

    const std::uint32_t ciphertext_size =
        static_cast<std::uint32_t>(ciphertext_len);

    file.write(
        MAGIC,
        static_cast<std::streamsize>(sizeof(MAGIC) - 1)
    );
    file.write(
        reinterpret_cast<const char*>(salt.data()),
        salt.size()
    );
    file.write(
        reinterpret_cast<const char*>(nonce.data()),
        nonce.size()
    );
    file.write(
        reinterpret_cast<const char*>(&public_size),
        sizeof(public_size)
    );
    file.write(
        reinterpret_cast<const char*>(&ciphertext_size),
        sizeof(ciphertext_size)
    );
    file.write(
        reinterpret_cast<const char*>(public_key_.data()),
        public_key_.size()
    );
    file.write(
        reinterpret_cast<const char*>(ciphertext.data()),
        ciphertext_len
    );
    file.write(
        reinterpret_cast<const char*>(tag.data()),
        tag.size()
    );

    file.flush();

    if (!file) {
        file.close();
        std::remove(temporary.c_str());
        return false;
    }

    file.close();

    if (std::rename(
            temporary.c_str(),
            path.c_str()) != 0) {
        std::remove(temporary.c_str());
        return false;
    }

    return true;
}

Wallet Wallet::load(
    const std::string& path,
    const std::string& password
) {
    if (password.empty())
        throw std::runtime_error(
            "wallet password required"
        );

    constexpr char MAGIC[] = "LARBWLT2";
    constexpr std::size_t MAGIC_SIZE = sizeof(MAGIC) - 1;
    constexpr std::size_t SALT_SIZE = 16;
    constexpr std::size_t NONCE_SIZE = 12;
    constexpr std::size_t TAG_SIZE = 16;
    constexpr int ITERATIONS = 200000;

    std::ifstream file(
        path,
        std::ios::binary
    );

    if (!file)
        throw std::runtime_error(
            "cannot open wallet file"
        );

    char magic[MAGIC_SIZE];
    file.read(
        magic,
        static_cast<std::streamsize>(MAGIC_SIZE)
    );

    if (!file ||
        !std::equal(
            magic,
            magic + MAGIC_SIZE,
            MAGIC)) {
        throw std::runtime_error(
            "invalid or unencrypted wallet file"
        );
    }

    std::array<unsigned char, SALT_SIZE> salt{};
    std::array<unsigned char, NONCE_SIZE> nonce{};

    file.read(
        reinterpret_cast<char*>(salt.data()),
        salt.size()
    );
    file.read(
        reinterpret_cast<char*>(nonce.data()),
        nonce.size()
    );

    std::uint32_t public_size = 0;
    std::uint32_t ciphertext_size = 0;

    file.read(
        reinterpret_cast<char*>(&public_size),
        sizeof(public_size)
    );
    file.read(
        reinterpret_cast<char*>(&ciphertext_size),
        sizeof(ciphertext_size)
    );

    if (!file ||
        public_size != PUBLIC_KEY_SIZE ||
        ciphertext_size != SECRET_KEY_SIZE) {
        throw std::runtime_error(
            "invalid encrypted wallet sizes"
        );
    }

    std::vector<uint8_t> public_key(public_size);
    std::vector<uint8_t> ciphertext(ciphertext_size);
    std::array<unsigned char, TAG_SIZE> tag{};

    file.read(
        reinterpret_cast<char*>(public_key.data()),
        public_key.size()
    );
    file.read(
        reinterpret_cast<char*>(ciphertext.data()),
        ciphertext.size()
    );
    file.read(
        reinterpret_cast<char*>(tag.data()),
        tag.size()
    );

    if (!file)
        throw std::runtime_error(
            "truncated encrypted wallet"
        );

    std::array<unsigned char, 32> key{};

    if (PKCS5_PBKDF2_HMAC(
            password.data(),
            static_cast<int>(password.size()),
            salt.data(),
            static_cast<int>(salt.size()),
            ITERATIONS,
            EVP_sha256(),
            static_cast<int>(key.size()),
            key.data()) != 1) {
        throw std::runtime_error(
            "wallet key derivation failed"
        );
    }

    std::vector<uint8_t> secret_key(
        SECRET_KEY_SIZE
    );

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::fill(key.begin(), key.end(), 0);
        throw std::runtime_error(
            "wallet cipher initialization failed"
        );
    }

    int len = 0;
    int plaintext_len = 0;
    bool ok = true;

    if (EVP_DecryptInit_ex(
            ctx,
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr) != 1)
        ok = false;

    if (ok && EVP_CIPHER_CTX_ctrl(
            ctx,
            EVP_CTRL_GCM_SET_IVLEN,
            NONCE_SIZE,
            nullptr) != 1)
        ok = false;

    if (ok && EVP_DecryptInit_ex(
            ctx,
            nullptr,
            nullptr,
            key.data(),
            nonce.data()) != 1)
        ok = false;

    if (ok && EVP_DecryptUpdate(
            ctx,
            nullptr,
            &len,
            reinterpret_cast<const unsigned char*>(MAGIC),
            MAGIC_SIZE) != 1)
        ok = false;

    if (ok && EVP_DecryptUpdate(
            ctx,
            nullptr,
            &len,
            public_key.data(),
            static_cast<int>(public_key.size())) != 1)
        ok = false;

    if (ok && EVP_DecryptUpdate(
            ctx,
            secret_key.data(),
            &len,
            ciphertext.data(),
            static_cast<int>(ciphertext.size())) != 1)
        ok = false;

    plaintext_len = len;

    if (ok && EVP_CIPHER_CTX_ctrl(
            ctx,
            EVP_CTRL_GCM_SET_TAG,
            TAG_SIZE,
            tag.data()) != 1)
        ok = false;

    if (ok && EVP_DecryptFinal_ex(
            ctx,
            secret_key.data() + plaintext_len,
            &len) != 1)
        ok = false;

    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    std::fill(key.begin(), key.end(), 0);

    if (!ok || plaintext_len != SECRET_KEY_SIZE)
        throw std::runtime_error(
            "invalid password or corrupted wallet"
        );

    Wallet wallet(
        std::move(public_key),
        std::move(secret_key)
    );

    const std::string proof_message =
        "LARB WALLET KEY PROOF";

    const std::string signature =
        wallet.sign(proof_message);

    if (!wallet.verify(
            proof_message,
            signature)) {
        throw std::runtime_error(
            "wallet keypair verification failed"
        );
    }

    return wallet;
}

} // namespace larb
