#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <string>

namespace secure_proto {
namespace crypto {

constexpr size_t X25519_KEY_SIZE = 32;
constexpr size_t CHACHA20_KEY_SIZE = 32;
constexpr size_t CHACHA20_NONCE_SIZE = 12;
constexpr size_t POLY1305_TAG_SIZE = 16;
constexpr size_t SHA256_DIGEST_SIZE = 32;

// Генерирует криптографически стойкие псевдослучайные байты
bool generate_random_bytes(uint8_t* out, size_t len) noexcept;

// X25519 (ECDH Curve25519)
void x25519_generate_keypair(uint8_t public_key[32], uint8_t private_key[32]) noexcept;
bool x25519_shared_secret(uint8_t shared_secret[32], const uint8_t private_key[32], const uint8_t peer_public_key[32]) noexcept;

// SHA-256 & HMAC-SHA256
void sha256(const uint8_t* data, size_t len, uint8_t out[32]) noexcept;
void hmac_sha256(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t out[32]) noexcept;

// HKDF-SHA256 (RFC 5869)
bool hkdf_extract_and_expand(
    const uint8_t* salt, size_t salt_len,
    const uint8_t* ikm, size_t ikm_len,
    const uint8_t* info, size_t info_len,
    uint8_t* okm, size_t okm_len
) noexcept;

// ChaCha20-Poly1305 AEAD (RFC 8439)
bool chacha20_poly1305_encrypt(
    const uint8_t key[32],
    const uint8_t nonce[12],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[16]
) noexcept;

bool chacha20_poly1305_decrypt(
    const uint8_t key[32],
    const uint8_t nonce[12],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[16],
    uint8_t* plaintext
) noexcept;

// Вспомогательное безопасное сравнение за постоянное время
bool constant_time_compare(const uint8_t* a, const uint8_t* b, size_t len) noexcept;
void secure_zero(void* ptr, size_t len) noexcept;

// Конвертация hex и base64
std::string to_hex(const uint8_t* data, size_t len) noexcept;
bool from_hex(const std::string& hex, uint8_t* out, size_t out_len) noexcept;
std::string base64_encode(const uint8_t* data, size_t len) noexcept;
std::vector<uint8_t> base64_decode(const std::string& b64) noexcept;

} // namespace crypto
} // namespace secure_proto
