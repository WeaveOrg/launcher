#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <cstring>
#include <type_traits>

namespace secure_proto {

class BinaryWriter;
class BinaryReader;

// ============================================================================
// Type Traits for Vector and Aggregate Detection
// ============================================================================
template <typename T>
struct is_std_vector : std::false_type {};

template <typename T, typename A>
struct is_std_vector<std::vector<T, A>> : std::true_type {};

template <typename T>
inline constexpr bool is_std_vector_v = is_std_vector<T>::value;

template <typename T>
inline constexpr bool is_custom_aggregate_v = 
    std::is_aggregate_v<T> && 
    !is_std_vector_v<T> && 
    !std::is_same_v<T, std::string>;

// Forward declarations for recursive struct serialization
template <typename T>
typename std::enable_if_t<is_custom_aggregate_v<T>, void>
serialize_struct(BinaryWriter& w, const T& obj) noexcept;

template <typename T>
typename std::enable_if_t<is_custom_aggregate_v<T>, bool>
deserialize_struct(BinaryReader& r, T& obj) noexcept;

// ============================================================================
// BinaryWriter - Serializes primitives, vectors and custom structures
// ============================================================================
class BinaryWriter {
public:
    BinaryWriter() noexcept {
        buffer_.reserve(256);
    }

    explicit BinaryWriter(size_t reserve_bytes) noexcept {
        buffer_.reserve(reserve_bytes);
    }

    void clear() noexcept {
        buffer_.clear();
    }

    void reserve(size_t extra_bytes) noexcept {
        buffer_.reserve(buffer_.size() + extra_bytes);
    }

    const uint8_t* data() const noexcept { return buffer_.data(); }
    size_t size() const noexcept { return buffer_.size(); }
    const std::vector<uint8_t>& buffer() const noexcept { return buffer_; }

    std::string to_string() const noexcept {
        return std::string(reinterpret_cast<const char*>(buffer_.data()), buffer_.size());
    }

    // --- Primitive Writers (Little-Endian) ---

    void write_u8(uint8_t val) noexcept { buffer_.push_back(val); }
    void write_i8(int8_t val) noexcept { buffer_.push_back(static_cast<uint8_t>(val)); }
    void write_bool(bool val) noexcept { buffer_.push_back(val ? 1 : 0); }

    void write_u16(uint16_t val) noexcept {
        buffer_.push_back(static_cast<uint8_t>(val & 0xFF));
        buffer_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    }

    void write_i16(int16_t val) noexcept { write_u16(static_cast<uint16_t>(val)); }

    void write_u32(uint32_t val) noexcept {
        buffer_.push_back(static_cast<uint8_t>(val & 0xFF));
        buffer_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        buffer_.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
        buffer_.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
    }

    void write_i32(int32_t val) noexcept { write_u32(static_cast<uint32_t>(val)); }

    void write_u64(uint64_t val) noexcept {
        for (int i = 0; i < 8; ++i) {
            buffer_.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
        }
    }

    void write_i64(int64_t val) noexcept { write_u64(static_cast<uint64_t>(val)); }

    void write_float(float val) noexcept {
        uint32_t bits = 0;
        std::memcpy(&bits, &val, sizeof(float));
        write_u32(bits);
    }

    void write_double(double val) noexcept {
        uint64_t bits = 0;
        std::memcpy(&bits, &val, sizeof(double));
        write_u64(bits);
    }

    // --- Strings and Blobs ---

    void write_string(const std::string& str) noexcept {
        write_u32(static_cast<uint32_t>(str.length()));
        if (!str.empty()) {
            buffer_.insert(buffer_.end(), str.begin(), str.end());
        }
    }

    void write_bytes(const uint8_t* src, size_t len) noexcept {
        write_u32(static_cast<uint32_t>(len));
        if (src && len > 0) {
            buffer_.insert(buffer_.end(), src, src + len);
        }
    }

    void write_bytes(const std::vector<uint8_t>& vec) noexcept {
        write_bytes(vec.data(), vec.size());
    }

    void write_raw(const void* src, size_t len) noexcept {
        if (src && len > 0) {
            const uint8_t* p = static_cast<const uint8_t*>(src);
            buffer_.insert(buffer_.end(), p, p + len);
        }
    }

    // --- Stream Operators for all primitive types ---
    BinaryWriter& operator<<(uint8_t v) noexcept { write_u8(v); return *this; }
    BinaryWriter& operator<<(int8_t v) noexcept { write_i8(v); return *this; }
    BinaryWriter& operator<<(bool v) noexcept { write_bool(v); return *this; }
    BinaryWriter& operator<<(uint16_t v) noexcept { write_u16(v); return *this; }
    BinaryWriter& operator<<(int16_t v) noexcept { write_i16(v); return *this; }
    BinaryWriter& operator<<(uint32_t v) noexcept { write_u32(v); return *this; }
    BinaryWriter& operator<<(int32_t v) noexcept { write_i32(v); return *this; }
    BinaryWriter& operator<<(uint64_t v) noexcept { write_u64(v); return *this; }
    BinaryWriter& operator<<(int64_t v) noexcept { write_i64(v); return *this; }
    BinaryWriter& operator<<(float v) noexcept { write_float(v); return *this; }
    BinaryWriter& operator<<(double v) noexcept { write_double(v); return *this; }
    BinaryWriter& operator<<(const std::string& v) noexcept { write_string(v); return *this; }
    BinaryWriter& operator<<(const char* v) noexcept { write_string(v ? v : ""); return *this; }

    // --- Stream Operator for ANY std::vector<T> (primitives, strings, custom structs) ---
    template <typename T>
    BinaryWriter& operator<<(const std::vector<T>& vec) noexcept {
        if constexpr (std::is_same_v<T, uint8_t>) {
            write_bytes(vec.data(), vec.size());
        } else {
            write_u32(static_cast<uint32_t>(vec.size()));
            for (const auto& elem : vec) {
                *this << elem;
            }
        }
        return *this;
    }

    // --- Stream Operator for ANY Custom Aggregate Struct ---
    template <typename T>
    typename std::enable_if_t<is_custom_aggregate_v<T>, BinaryWriter&>
    operator<<(const T& obj) noexcept {
        serialize_struct(*this, obj);
        return *this;
    }

private:
    std::vector<uint8_t> buffer_;
};

// ============================================================================
// BinaryReader - Zero-exception safe binary stream deserializer
// ============================================================================
class BinaryReader {
public:
    BinaryReader(const uint8_t* data, size_t size) noexcept
        : data_(data), size_(size), offset_(0), error_(false) {}

    explicit BinaryReader(const std::vector<uint8_t>& vec) noexcept
        : data_(vec.data()), size_(vec.size()), offset_(0), error_(false) {}

    explicit BinaryReader(const std::string& str) noexcept
        : data_(reinterpret_cast<const uint8_t*>(str.data())), size_(str.size()), offset_(0), error_(false) {}

    bool ok() const noexcept { return !error_; }
    bool has_error() const noexcept { return error_; }
    size_t offset() const noexcept { return offset_; }
    size_t size() const noexcept { return size_; }
    size_t remaining() const noexcept { return (offset_ < size_) ? (size_ - offset_) : 0; }

    uint8_t read_u8() noexcept {
        if (offset_ + 1 > size_) { error_ = true; return 0; }
        return data_[offset_++];
    }

    int8_t read_i8() noexcept { return static_cast<int8_t>(read_u8()); }
    bool read_bool() noexcept { return read_u8() != 0; }

    uint16_t read_u16() noexcept {
        if (offset_ + 2 > size_) { error_ = true; return 0; }
        uint16_t val = static_cast<uint16_t>(data_[offset_]) |
                       (static_cast<uint16_t>(data_[offset_ + 1]) << 8);
        offset_ += 2;
        return val;
    }

    int16_t read_i16() noexcept { return static_cast<int16_t>(read_u16()); }

    uint32_t read_u32() noexcept {
        if (offset_ + 4 > size_) { error_ = true; return 0; }
        uint32_t val = static_cast<uint32_t>(data_[offset_]) |
                       (static_cast<uint32_t>(data_[offset_ + 1]) << 8) |
                       (static_cast<uint32_t>(data_[offset_ + 2]) << 16) |
                       (static_cast<uint32_t>(data_[offset_ + 3]) << 24);
        offset_ += 4;
        return val;
    }

    int32_t read_i32() noexcept { return static_cast<int32_t>(read_u32()); }

    uint64_t read_u64() noexcept {
        if (offset_ + 8 > size_) { error_ = true; return 0; }
        uint64_t val = 0;
        for (int i = 0; i < 8; ++i) {
            val |= static_cast<uint64_t>(data_[offset_ + i]) << (i * 8);
        }
        offset_ += 8;
        return val;
    }

    int64_t read_i64() noexcept { return static_cast<int64_t>(read_u64()); }

    float read_float() noexcept {
        uint32_t bits = read_u32();
        float val = 0.0f;
        std::memcpy(&val, &bits, sizeof(float));
        return val;
    }

    double read_double() noexcept {
        uint64_t bits = read_u64();
        double val = 0.0;
        std::memcpy(&val, &bits, sizeof(double));
        return val;
    }

    std::string read_string() noexcept {
        uint32_t len = read_u32();
        if (error_ || offset_ + len > size_) {
            error_ = true;
            return "";
        }
        std::string s(reinterpret_cast<const char*>(data_ + offset_), len);
        offset_ += len;
        return s;
    }

    std::vector<uint8_t> read_bytes() noexcept {
        uint32_t len = read_u32();
        if (error_ || offset_ + len > size_) {
            error_ = true;
            return {};
        }
        std::vector<uint8_t> v(data_ + offset_, data_ + offset_ + len);
        offset_ += len;
        return v;
    }

    bool read_raw(void* dst, size_t len) noexcept {
        if (error_ || offset_ + len > size_) {
            error_ = true;
            return false;
        }
        if (dst && len > 0) {
            std::memcpy(dst, data_ + offset_, len);
        }
        offset_ += len;
        return true;
    }

    // --- Stream Operators for primitives ---
    BinaryReader& operator>>(uint8_t& v) noexcept { v = read_u8(); return *this; }
    BinaryReader& operator>>(int8_t& v) noexcept { v = read_i8(); return *this; }
    BinaryReader& operator>>(bool& v) noexcept { v = read_bool(); return *this; }
    BinaryReader& operator>>(uint16_t& v) noexcept { v = read_u16(); return *this; }
    BinaryReader& operator>>(int16_t& v) noexcept { v = read_i16(); return *this; }
    BinaryReader& operator>>(uint32_t& v) noexcept { v = read_u32(); return *this; }
    BinaryReader& operator>>(int32_t& v) noexcept { v = read_i32(); return *this; }
    BinaryReader& operator>>(uint64_t& v) noexcept { v = read_u64(); return *this; }
    BinaryReader& operator>>(int64_t& v) noexcept { v = read_i64(); return *this; }
    BinaryReader& operator>>(float& v) noexcept { v = read_float(); return *this; }
    BinaryReader& operator>>(double& v) noexcept { v = read_double(); return *this; }
    BinaryReader& operator>>(std::string& v) noexcept { v = read_string(); return *this; }

    // --- Stream Operator for ANY std::vector<T> (primitives, strings, custom structs) ---
    template <typename T>
    BinaryReader& operator>>(std::vector<T>& vec) noexcept {
        uint32_t len = read_u32();
        if (error_ || len > 200000000) {
            error_ = true;
            return *this;
        }
        if constexpr (std::is_same_v<T, uint8_t>) {
            if (offset_ + len > size_) {
                error_ = true;
                return *this;
            }
            vec.assign(data_ + offset_, data_ + offset_ + len);
            offset_ += len;
        } else {
            vec.clear();
            vec.reserve(len);
            for (uint32_t i = 0; i < len && ok(); ++i) {
                T elem{};
                *this >> elem;
                vec.push_back(std::move(elem));
            }
        }
        return *this;
    }

    // --- Stream Operator for ANY Custom Aggregate Struct ---
    template <typename T>
    typename std::enable_if_t<is_custom_aggregate_v<T>, BinaryReader&>
    operator>>(T& obj) noexcept {
        deserialize_struct(*this, obj);
        return *this;
    }

private:
    const uint8_t* data_;
    size_t size_;
    size_t offset_;
    bool error_;
};

// ============================================================================
// Automatic C++17 Aggregate Reflection (Zero-Boilerplate Struct Visitor)
// ============================================================================
namespace detail {

struct UniversalType {
    template <typename T>
    operator T() const;
};

template <typename T, typename = void, typename... Args>
struct count_fields : std::integral_constant<size_t, sizeof...(Args) - 1> {};

template <typename T, typename... Args>
struct count_fields<T, std::void_t<decltype(T{Args{}...})>, Args...>
    : count_fields<T, void, Args..., UniversalType> {};

template <typename T>
constexpr size_t get_field_count() {
    return count_fields<T, void, UniversalType>::value;
}

template <typename T, typename F>
void visit_fields(T& obj, F&& func) noexcept {
    constexpr size_t count = get_field_count<T>();

    if constexpr (count == 0) {
        // Empty struct
    } else if constexpr (count == 1) {
        auto& [a] = obj;
        func(a);
    } else if constexpr (count == 2) {
        auto& [a, b] = obj;
        func(a, b);
    } else if constexpr (count == 3) {
        auto& [a, b, c] = obj;
        func(a, b, c);
    } else if constexpr (count == 4) {
        auto& [a, b, c, d] = obj;
        func(a, b, c, d);
    } else if constexpr (count == 5) {
        auto& [a, b, c, d, e] = obj;
        func(a, b, c, d, e);
    } else if constexpr (count == 6) {
        auto& [a, b, c, d, e, f] = obj;
        func(a, b, c, d, e, f);
    } else if constexpr (count == 7) {
        auto& [a, b, c, d, e, f, g] = obj;
        func(a, b, c, d, e, f, g);
    } else if constexpr (count == 8) {
        auto& [a, b, c, d, e, f, g, h] = obj;
        func(a, b, c, d, e, f, g, h);
    } else if constexpr (count == 9) {
        auto& [a, b, c, d, e, f, g, h, i] = obj;
        func(a, b, c, d, e, f, g, h, i);
    } else if constexpr (count == 10) {
        auto& [a, b, c, d, e, f, g, h, i, j] = obj;
        func(a, b, c, d, e, f, g, h, i, j);
    } else if constexpr (count == 11) {
        auto& [a, b, c, d, e, f, g, h, i, j, k] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k);
    } else if constexpr (count == 12) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l);
    } else if constexpr (count == 13) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m);
    } else if constexpr (count == 14) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
    } else if constexpr (count == 15) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    } else if constexpr (count == 16) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
    } else if constexpr (count == 17) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q);
    } else if constexpr (count == 18) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r);
    } else if constexpr (count == 19) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s);
    } else if constexpr (count == 20) {
        auto& [a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t] = obj;
        func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t);
    }
}

} // namespace detail

template <typename T>
typename std::enable_if_t<is_custom_aggregate_v<T>, void>
serialize_struct(BinaryWriter& w, const T& obj) noexcept {
    detail::visit_fields(const_cast<T&>(obj), [&](auto&... fields) {
        (w << ... << fields);
    });
}

template <typename T>
typename std::enable_if_t<is_custom_aggregate_v<T>, bool>
deserialize_struct(BinaryReader& r, T& obj) noexcept {
    detail::visit_fields(obj, [&](auto&... fields) {
        (r >> ... >> fields);
    });
    return r.ok();
}

} // namespace secure_proto
