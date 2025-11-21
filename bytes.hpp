#pragma once

/**
 * @file bytes.hpp
 * @brief Fixed-size byte array dengan operasi bitwise
 * @version 1.1.0
 * 
 * Container compile-time untuk manipulasi bit-level.
 * Dioptimasi untuk operasi bitwise dan cache efficiency.
 */

#include <bit>
#include <cstdint>
#include <cstring>

namespace zuu {

/**
 * @brief Fixed-size byte array dengan operasi bitwise
 * @tparam N Jumlah byte (harus > 0)
 * 
 * Memory layout: aligned 16-byte untuk SIMD compatibility
 * 
 * @note Semua operasi constexpr dan noexcept
 * @note Little-endian byte order
 */
template <size_t N>
requires (N > 0)
class bytes {
public:
    // ============= Type Aliases =============
    using byte_t = uint8_t;
    using size_type = size_t;
    using value_type = byte_t;
    using pointer = byte_t*;
    using const_pointer = const byte_t*;
    using reference = byte_t&;
    using const_reference = const byte_t&;

    static constexpr size_type byte_count = N;
    static constexpr size_type bit_count = N * 8;

private:
    alignas(N >= 16 ? 16 : (N >= 8 ? 8 : (N >= 4 ? 4 : 1))) 
    byte_t data_[N]{};

public:
    // ============= Constructors =============
    
    constexpr bytes() noexcept = default;
    constexpr bytes(const bytes&) noexcept = default;
    constexpr bytes(bytes&&) noexcept = default;
    constexpr bytes& operator=(const bytes&) noexcept = default;
    constexpr bytes& operator=(bytes&&) noexcept = default;

    /** @brief Construct dari array */
    constexpr bytes(const byte_t (&data)[N]) noexcept {
        for (size_type i = 0; i < N; ++i) data_[i] = data[i];
    }

    /** @brief Construct dari pointer + length */
    constexpr bytes(const byte_t* data, size_type len) noexcept {
        const size_type n = len < N ? len : N;
        for (size_type i = 0; i < n; ++i) data_[i] = data[i];
    }

    /** @brief Construct dari integer (little-endian) */
    template <typename IntT>
    requires std::is_integral_v<IntT>
    constexpr explicit bytes(IntT value) noexcept {
        constexpr size_type copy = sizeof(IntT) < N ? sizeof(IntT) : N;
        if constexpr (copy <= 8) {
            for (size_type i = 0; i < copy; ++i) {
                data_[i] = static_cast<byte_t>(value >> (i * 8));
            }
        } else {
            std::memcpy(data_, &value, copy);
        }
    }

    /** @brief Fill constructor */
    constexpr explicit bytes(byte_t fill_value) noexcept {
        for (size_type i = 0; i < N; ++i) data_[i] = fill_value;
    }

    // ============= Element Access =============

    [[nodiscard]] constexpr reference operator[](size_type i) noexcept { 
        return data_[i < N ? i : N - 1]; 
    }
    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept { 
        return data_[i < N ? i : N - 1]; 
    }
    [[nodiscard]] constexpr reference at(size_type i) noexcept { return data_[i]; }
    [[nodiscard]] constexpr const_reference at(size_type i) const noexcept { return data_[i]; }
    [[nodiscard]] constexpr reference front() noexcept { return data_[0]; }
    [[nodiscard]] constexpr const_reference front() const noexcept { return data_[0]; }
    [[nodiscard]] constexpr reference back() noexcept { return data_[N - 1]; }
    [[nodiscard]] constexpr const_reference back() const noexcept { return data_[N - 1]; }
    [[nodiscard]] constexpr pointer data() noexcept { return data_; }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return data_; }

    // ============= Capacity =============

    [[nodiscard]] static constexpr size_type size() noexcept { return N; }
    [[nodiscard]] static constexpr size_type bit_size() noexcept { return N * 8; }
    [[nodiscard]] static constexpr bool empty() noexcept { return false; }

    // ============= Iterators =============

    [[nodiscard]] constexpr pointer begin() noexcept { return data_; }
    [[nodiscard]] constexpr pointer end() noexcept { return data_ + N; }
    [[nodiscard]] constexpr const_pointer begin() const noexcept { return data_; }
    [[nodiscard]] constexpr const_pointer end() const noexcept { return data_ + N; }
    [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return data_; }
    [[nodiscard]] constexpr const_pointer cend() const noexcept { return data_ + N; }

    // ============= Bitwise Operations =============

    [[nodiscard]] constexpr bytes operator|(const bytes& o) const noexcept {
        bytes r;
        for (size_type i = 0; i < N; ++i) r.data_[i] = data_[i] | o.data_[i];
        return r;
    }

    [[nodiscard]] constexpr bytes operator&(const bytes& o) const noexcept {
        bytes r;
        for (size_type i = 0; i < N; ++i) r.data_[i] = data_[i] & o.data_[i];
        return r;
    }

    [[nodiscard]] constexpr bytes operator^(const bytes& o) const noexcept {
        bytes r;
        for (size_type i = 0; i < N; ++i) r.data_[i] = data_[i] ^ o.data_[i];
        return r;
    }

    [[nodiscard]] constexpr bytes operator~() const noexcept {
        bytes r;
        for (size_type i = 0; i < N; ++i) r.data_[i] = ~data_[i];
        return r;
    }

    // ============= Shift Operations =============

    [[nodiscard]] constexpr bytes operator<<(size_type bits) const noexcept {
        if (bits == 0) return *this;
        if (bits >= bit_count) return bytes{};
        
        bytes r;
        const size_type byte_sh = bits / 8;
        const size_type bit_sh = bits % 8;

        if (bit_sh == 0) {
            for (size_type i = byte_sh; i < N; ++i) 
                r.data_[i] = data_[i - byte_sh];
        } else {
            byte_t carry = 0;
            for (size_type i = 0; i < N - byte_sh; ++i) {
                r.data_[i + byte_sh] = (data_[i] << bit_sh) | carry;
                carry = data_[i] >> (8 - bit_sh);
            }
        }
        return r;
    }

    [[nodiscard]] constexpr bytes operator>>(size_type bits) const noexcept {
        if (bits == 0) return *this;
        if (bits >= bit_count) return bytes{};
        
        bytes r;
        const size_type byte_sh = bits / 8;
        const size_type bit_sh = bits % 8;

        if (bit_sh == 0) {
            for (size_type i = 0; i < N - byte_sh; ++i) 
                r.data_[i] = data_[i + byte_sh];
        } else {
            byte_t carry = 0;
            for (size_type i = N - byte_sh; i-- > 0;) {
                r.data_[i] = (data_[i + byte_sh] >> bit_sh) | carry;
                carry = data_[i + byte_sh] << (8 - bit_sh);
            }
        }
        return r;
    }

    // ============= Compound Assignment =============

    constexpr bytes& operator|=(const bytes& o) noexcept { return *this = *this | o; }
    constexpr bytes& operator&=(const bytes& o) noexcept { return *this = *this & o; }
    constexpr bytes& operator^=(const bytes& o) noexcept { return *this = *this ^ o; }
    constexpr bytes& operator<<=(size_type n) noexcept { return *this = *this << n; }
    constexpr bytes& operator>>=(size_type n) noexcept { return *this = *this >> n; }

    // ============= Bit Manipulation =============

    constexpr void set_bit(size_type pos) noexcept {
        if (pos < bit_count) data_[pos / 8] |= (1u << (pos % 8));
    }

    constexpr void clear_bit(size_type pos) noexcept {
        if (pos < bit_count) data_[pos / 8] &= ~(1u << (pos % 8));
    }

    constexpr void toggle_bit(size_type pos) noexcept {
        if (pos < bit_count) data_[pos / 8] ^= (1u << (pos % 8));
    }

    [[nodiscard]] constexpr bool test_bit(size_type pos) const noexcept {
        return pos < bit_count && (data_[pos / 8] & (1u << (pos % 8))) != 0;
    }

    [[nodiscard]] constexpr size_type popcount() const noexcept {
        size_type c = 0;
        for (size_type i = 0; i < N; ++i) c += std::popcount(data_[i]);
        return c;
    }

    // ============= Rotation =============

    [[nodiscard]] constexpr bytes rotate_left(size_type n) const noexcept {
        n %= bit_count;
        return (*this << n) | (*this >> (bit_count - n));
    }

    [[nodiscard]] constexpr bytes rotate_right(size_type n) const noexcept {
        n %= bit_count;
        return (*this >> n) | (*this << (bit_count - n));
    }

    // ============= Conversion =============

    template <typename IntT>
    requires std::is_integral_v<IntT>
    [[nodiscard]] constexpr IntT to_int() const noexcept {
        IntT r = 0;
        constexpr size_type copy = sizeof(IntT) < N ? sizeof(IntT) : N;
        for (size_type i = 0; i < copy; ++i) {
            r |= static_cast<IntT>(data_[i]) << (i * 8);
        }
        return r;
    }

    // ============= Modifiers =============

    constexpr void fill(byte_t v) noexcept {
        for (size_type i = 0; i < N; ++i) data_[i] = v;
    }

    constexpr void clear() noexcept { fill(0); }

    [[nodiscard]] constexpr bytes reverse() const noexcept {
        bytes r;
        for (size_type i = 0; i < N; ++i) r.data_[i] = data_[N - 1 - i];
        return r;
    }

    // ============= Comparison =============

    [[nodiscard]] constexpr bool operator==(const bytes&) const noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(const bytes&) const noexcept = default;
};

// Deduction guide
template <size_t N>
bytes(const unsigned char (&)[N]) -> bytes<N>;

} // namespace zuu
