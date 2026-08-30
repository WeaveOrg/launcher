#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <string>
#include <utility>

namespace secure_proto {
namespace secret {

// 1. Constexpr FNV-1a 64-bit Hash for generating unique compile-time seeds
constexpr uint64_t fnv1a_64(const char* str, size_t len, uint64_t seed = 0xcbf29ce484222325ULL) noexcept {
    uint64_t hash = seed;
    for (size_t i = 0; i < len; ++i) {
        hash ^= static_cast<uint64_t>(str[i]);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

// 2. Constexpr Pseudo-Random Number Generator (Xorshift64Star)
class ConstexprPRNG {
public:
    constexpr explicit ConstexprPRNG(uint64_t seed) noexcept
        : state_(seed == 0 ? 0x8a5cd789635d2dffULL : seed) {}

    constexpr uint64_t next64() noexcept {
        uint64_t x = state_;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        state_ = x;
        return x * 0x2545F4914F6CDD1DULL;
    }

    constexpr uint8_t next8() noexcept {
        return static_cast<uint8_t>(next64() & 0xFF);
    }

private:
    uint64_t state_;
};

// 3. Compile-time non-linear encryption transformation
// Uses multi-round combined non-linear operations (XOR -> ADD -> XOR)
template <size_t N, uint64_t Seed>
class EncryptedStringStorage {
public:
    template <size_t... Is>
    constexpr explicit EncryptedStringStorage(const char* str, std::index_sequence<Is...>) noexcept
        : data_{ encrypt_byte(str[Is], Is)... } {}

    static constexpr uint8_t get_key_a(size_t index) noexcept {
        ConstexprPRNG prng(Seed ^ (index * 0x9e3779b97f4a7c15ULL));
        return prng.next8();
    }

    static constexpr uint8_t get_key_b(size_t index) noexcept {
        ConstexprPRNG prng((Seed >> 32) ^ (index * 0xbf58476d1ce4e5b9ULL));
        return prng.next8();
    }

    static constexpr char encrypt_byte(char c, size_t index) noexcept {
        uint8_t b = static_cast<uint8_t>(c);
        uint8_t ka = get_key_a(index);
        uint8_t kb = get_key_b(index);
        // Non-linear cipher: (byte ^ ka) + kb
        uint8_t enc = static_cast<uint8_t>((b ^ ka) + kb);
        return static_cast<char>(enc);
    }

    // Stack-allocated RAII decrypted string view with auto-zeroing on destruction
    class StackDecrypted {
    public:
        explicit StackDecrypted(const char* enc_data) noexcept {
            for (size_t i = 0; i < N; ++i) {
                uint8_t enc = static_cast<uint8_t>(enc_data[i]);
                uint8_t ka = get_key_a(i);
                uint8_t kb = get_key_b(i);
                // Inverse transformation: (enc - kb) ^ ka
                uint8_t dec = static_cast<uint8_t>((enc - kb) ^ ka);
                buffer_[i] = static_cast<char>(dec);
            }
        }

        ~StackDecrypted() noexcept {
            // Volatile zeroing memory to prevent plaintext from remaining on the stack
            volatile char* p = buffer_;
            for (size_t i = 0; i < N; ++i) {
                p[i] = 0;
            }
        }

        StackDecrypted(const StackDecrypted&) = delete;
        StackDecrypted& operator=(const StackDecrypted&) = delete;

        const char* c_str() const noexcept { return buffer_; }
        const char* data() const noexcept { return buffer_; }
        constexpr size_t size() const noexcept { return N > 0 ? N - 1 : 0; }
        std::string str() const { return std::string(buffer_, size()); }

        operator std::string() const { return str(); }
        operator const char*() const noexcept { return buffer_; }

    private:
        char buffer_[N];
    };

    StackDecrypted decrypt() const noexcept {
        return StackDecrypted(data_);
    }

private:
    char data_[N];
};

} // namespace secret
} // namespace secure_proto

// Helper seed generation macro mixing compile time, file, line and counter
#define SQP_STRING_SEED \
    (::secure_proto::secret::fnv1a_64(__FILE__, sizeof(__FILE__)) ^ \
    (::secure_proto::secret::fnv1a_64(__DATE__ __TIME__, sizeof(__DATE__ __TIME__)) + (__LINE__ * 0x9e3779b97f4a7c15ULL) + (__COUNTER__ * 0x517cc1b727220a95ULL)))

// Main macro for compile-time string protection:
// Example usage:
//   auto domain = SQP_PROTECT("api.yourdomain.com");
//   client.connect(domain.c_str());
#define SQP_PROTECT(str) \
    ([]() -> auto { \
        constexpr size_t len = sizeof(str); \
        constexpr uint64_t seed = SQP_STRING_SEED; \
        static constexpr auto encrypted = \
            ::secure_proto::secret::EncryptedStringStorage<len, seed>( \
                str, std::make_index_sequence<len>{} \
            ); \
        return encrypted.decrypt(); \
    }())
