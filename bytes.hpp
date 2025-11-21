#pragma once

/**
 * @file bytes.hpp
 * @brief Fixed-size byte array dengan operasi bitwise
 * @version 1.1.0
 * 
 * Container compile-time untuk manipulasi bit-level.
 * Dioptimasi untuk operasi bitwise dan cache efficiency.
 */

#include "endian.hpp"
#include <algorithm>
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

    // ============= Endian Conversion =============

    /**
     * @brief Convert ke little-endian (reverse jika native big-endian)
     * @return bytes dalam little-endian order
     * @note No-op pada little-endian systems
     */
    [[nodiscard]] constexpr bytes to_little_endian() const noexcept {
        if constexpr (is_little_endian) {
            return *this;
        } else {
            return reverse();
        }
    }

    /**
     * @brief Convert ke big-endian (reverse jika native little-endian)
     * @return bytes dalam big-endian order
     * @note No-op pada big-endian systems
     */
    [[nodiscard]] constexpr bytes to_big_endian() const noexcept {
        if constexpr (is_big_endian) {
            return *this;
        } else {
            return reverse();
        }
    }

    /**
     * @brief Convert ke network byte order (big-endian)
     * @return bytes dalam network order
     */
    [[nodiscard]] constexpr bytes to_network() const noexcept {
        return to_big_endian();
    }

    /**
     * @brief Interpret sebagai little-endian, convert ke native
     * @return bytes dalam native order
     */
    [[nodiscard]] constexpr bytes from_little_endian() const noexcept {
        return to_little_endian(); // Symmetric operation
    }

    /**
     * @brief Interpret sebagai big-endian, convert ke native
     * @return bytes dalam native order
     */
    [[nodiscard]] constexpr bytes from_big_endian() const noexcept {
        return to_big_endian(); // Symmetric operation
    }

    /**
     * @brief Interpret sebagai network order, convert ke native
     * @return bytes dalam native order
     */
    [[nodiscard]] constexpr bytes from_network() const noexcept {
        return from_big_endian();
    }

    /**
     * @brief Convert ke endianness tertentu (runtime)
     * @param target Target endianness
     * @return bytes dalam target order
     */
    [[nodiscard]] constexpr bytes to_endian(endian_t target) const noexcept {
        if (target == native_endian) {
            return *this;
        } else {
            return reverse();
        }
    }

    /**
     * @brief Convert dari endianness tertentu ke native (runtime)
     * @param source Source endianness
     * @return bytes dalam native order
     */
    [[nodiscard]] constexpr bytes from_endian(endian_t source) const noexcept {
        return to_endian(source); // Symmetric
    }

    /**
     * @brief Reverse bytes in-place
     */
    constexpr void swap_bytes() noexcept {
        for (size_type i = 0; i < N / 2; ++i) {
            byte_t temp = data_[i];
            data_[i] = data_[N - 1 - i];
            data_[N - 1 - i] = temp;
        }
    }

    /**
     * @brief Convert to little-endian in-place
     * @note No-op pada little-endian systems
     */
    constexpr void make_little_endian() noexcept {
        if constexpr (!is_little_endian) {
            swap_bytes();
        }
    }

    /**
     * @brief Convert to big-endian in-place
     * @note No-op pada big-endian systems
     */
    constexpr void make_big_endian() noexcept {
        if constexpr (!is_big_endian) {
            swap_bytes();
        }
    }

    // ============= Integer Conversion with Endian =============

    /**
     * @brief Convert ke integer dengan endian tertentu
     * @tparam IntT Target integer type
     * @param source_endian Endianness dari stored bytes
     * @return Integer value dalam native byte order
     */
    template <typename IntT>
    requires std::is_integral_v<IntT>
    [[nodiscard]] constexpr IntT to_int(endian_t source_endian) const noexcept {
        auto native_bytes = from_endian(source_endian);
        return native_bytes.template to_int<IntT>();
    }

    /**
     * @brief Create bytes dari integer dengan target endian
     * @tparam IntT Source integer type
     * @param value Integer value (native byte order)
     * @param target_endian Target byte order untuk storage
     */
    template <typename IntT>
    requires std::is_integral_v<IntT>
    [[nodiscard]] static constexpr bytes from_int(IntT value, endian_t target_endian) noexcept {
        bytes result(value);
        if (target_endian != native_endian) {
            result.swap_bytes();
        }
        return result;
    }

    /**
     * @brief Create bytes dari little-endian integer
     */
    template <typename IntT>
    requires std::is_integral_v<IntT>
    [[nodiscard]] static constexpr bytes from_little_endian_int(IntT value) noexcept {
        return from_int(zuu::to_little_endian(value), endian_t::little);
    }

    /**
     * @brief Create bytes dari big-endian integer
     */
    template <typename IntT>
    requires std::is_integral_v<IntT>
    [[nodiscard]] static constexpr bytes from_big_endian_int(IntT value) noexcept {
        return from_int(zuu::to_big_endian(value), endian_t::big);
    }

    // ============= Comparison =============

    [[nodiscard]] constexpr bool operator==(const bytes&) const noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(const bytes&) const noexcept = default;
};

// Deduction guide
template <size_t N>
bytes(const unsigned char (&)[N]) -> bytes<N>;

} // namespace zuu
