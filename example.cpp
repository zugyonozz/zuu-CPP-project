/**
 * @file example.cpp
 * @brief Contoh penggunaan zuu::generic dan utilities
 */

#include "generic.hpp"
#include "bytes.hpp"
#include <iomanip>
#include <iostream>
#include <string>

// Custom trivial type untuk testing
struct Point {
    float x, y;
    constexpr bool operator==(const Point&) const = default;
};

int main() {
    using namespace zuu;
    
    std::cout << "=== zuu::generic Examples ===\n\n";

    // ============= Basic Usage =============
    
    std::cout << "1. Basic Construction:\n";
    generic<int, double, Point> g1(42);
    generic<int, double, Point> g2(3.14159);
    generic<int, double, Point> g3(Point{1.0f, 2.0f});
    
    std::cout << "   g1 holds int: " << g1.holds<int>() << "\n";
    std::cout << "   g2 holds double: " << g2.holds<double>() << "\n";
    std::cout << "   g3 holds Point: " << g3.holds<Point>() << "\n\n";

    // ============= Safe Access with get<T>() =============
    
    std::cout << "2. Safe Access (get<T>):\n";
    std::cout << "   g1.get<int>() = " << g1.get<int>() << "\n";
    std::cout << "   g2.get<double>() = " << g2.get<double>() << "\n";
    
    auto pt = g3.get<Point>();
    std::cout << "   g3.get<Point>() = {" << pt.x << ", " << pt.y << "}\n\n";

    // ============= Pointer Access with get_if<T>() =============
    
    std::cout << "3. Pointer Access (get_if<T>):\n";
    if (auto* p = g1.get_if<int>()) {
        std::cout << "   g1 contains int: " << *p << "\n";
    }
    if (auto* p = g1.get_if<double>()) {
        std::cout << "   g1 contains double\n";  // won't print
    } else {
        std::cout << "   g1 does NOT contain double\n";
    }
    std::cout << "\n";

    // ============= Visit Pattern =============
    
    std::cout << "4. Visit Pattern:\n";
    
    // Visit dengan return value
    auto result = g2.visit([](auto& val) -> double {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, Point>) {
            return val.x + val.y;
        } else {
            return static_cast<double>(val);
        }
    });
    std::cout << "   g2.visit() = " << result << "\n";

    // Visit void untuk side effects
    g3.visit_void([](auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, Point>) {
            std::cout << "   g3 is Point: {" << val.x << ", " << val.y << "}\n";
        }
    });
    std::cout << "\n";

    // ============= Overload Pattern =============
    
    std::cout << "5. Overload Pattern:\n";
    g1.visit_void(overload{
        [](int i)    { std::cout << "   int: " << i << "\n"; },
        [](double d) { std::cout << "   double: " << d << "\n"; },
        [](Point p)  { std::cout << "   Point: {" << p.x << ", " << p.y << "}\n"; }
    });
    std::cout << "\n";

    // ============= Emplace =============
    
    std::cout << "6. Emplace:\n";
    generic<int, double, Point> g4;
    std::cout << "   g4 has_value (before): " << g4.has_value() << "\n";
    
    g4.emplace<Point>(5.0f, 10.0f);
    std::cout << "   g4 has_value (after): " << g4.has_value() << "\n";
    std::cout << "   g4.get<Point>() = {" << g4.get<Point>().x << ", " 
              << g4.get<Point>().y << "}\n\n";

    // ============= Assignment =============
    
    std::cout << "7. Assignment:\n";
    g4 = 999;  // Change type to int
    std::cout << "   After g4 = 999: holds<int> = " << g4.holds<int>() 
              << ", value = " << g4.get<int>() << "\n\n";

    // ============= Reset =============
    
    std::cout << "8. Reset:\n";
    g4.reset();
    std::cout << "   After reset: has_value = " << g4.has_value() << "\n\n";

    // ============= Comparison =============
    
    std::cout << "9. Comparison:\n";
    generic<int, double> a(42);
    generic<int, double> b(42);
    generic<int, double> c(42.0);
    
    std::cout << "   a(42) == b(42): " << (a == b) << "\n";
    std::cout << "   a(42) == c(42.0): " << (a == c) << "\n\n";

    // ============= Type Info =============
    
    std::cout << "10. Type Info (compile-time):\n";
    using MyGeneric = generic<int, double, Point>;
    std::cout << "    type_count: " << MyGeneric::type_count << "\n";
    std::cout << "    max_size: " << MyGeneric::max_size << " bytes\n";
    std::cout << "    max_align: " << MyGeneric::max_align << " bytes\n";
    std::cout << "    storage_size: " << MyGeneric::storage_size() << " bytes\n";
    std::cout << "    sizeof(generic): " << sizeof(MyGeneric) << " bytes\n\n";

    // ============= Composer Usage =============
    
    std::cout << "11. Composer (type punning):\n";
    composer<int> d(0x12345678);
    std::cout << "    Value: 0x" << std::hex << d.value() << std::dec << "\n";
    std::cout << "    Bytes: ";
    for (auto b : d) std::cout << std::hex << (int)b << " ";
    std::cout << std::dec << "\n\n";

    // ============= Bytes Usage =============
    
    std::cout << "12. Bytes (bitwise ops):\n";
    bytes<4> b1(0xF0F0F0F0u);
    bytes<4> b2(0x0F0F0F0Fu);
    auto b3 = b1 | b2;
    
    std::cout << "    b1 | b2 = 0x" << std::hex << b3.to_int<uint32_t>() 
              << std::dec << "\n";
    std::cout << "    popcount(b3) = " << b3.popcount() << " bits\n\n";

    // ============= Endian Conversion =============
    
    std::cout << "13. Endian Conversion:\n";
    std::cout << "    Native endian: " << (is_little_endian ? "little" : "big") << "\n";
    
    // Composer endian
    composer<uint32_t> c2(0x12345678u);
    std::cout << "    Original: 0x" << std::hex << c2.value() << "\n";
    
    auto c2_le = c2.to_little_endian();
    auto c2_be = c2.to_big_endian();
    std::cout << "    to_little_endian: 0x" << c2_le.value() << "\n";
    std::cout << "    to_big_endian:    0x" << c2_be.value() << "\n";
    
    // Bytes as hex
    std::cout << "    Original bytes:   ";
    for (auto byte : c2) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    std::cout << "\n";
    
    std::cout << "    Big-endian bytes: ";
    for (auto byte : c2_be) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    std::cout << std::dec << "\n\n";

    // Bytes endian
    std::cout << "14. Bytes Endian:\n";
    bytes<4> b4(0xAABBCCDDu);
    std::cout << "    Original: ";
    for (auto byte : b4) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    std::cout << "\n";
    
    auto b4_be = b4.to_big_endian();
    std::cout << "    to_big_endian: ";
    for (auto byte : b4_be) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    std::cout << "\n";
    
    auto b4_net = b4.to_network();
    std::cout << "    to_network:    ";
    for (auto byte : b4_net) std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    std::cout << std::dec << "\n\n";

    // Network byte order (common use case)
    std::cout << "15. Network Byte Order:\n";
    uint16_t port = 8080;
    auto port_net = hton(port);
    std::cout << "    Port " << port << " -> network order: " << port_net << "\n";
    std::cout << "    Back to host: " << ntoh(port_net) << "\n\n";

    // Integration: composer + bytes + endian
    std::cout << "16. Integration Example:\n";
    uint32_t ip_addr = 0xC0A80001; // 192.168.0.1
    composer<uint32_t> ip_comp(ip_addr);
    auto ip_bytes = ip_comp.to_network().as_bytes();
    
    std::cout << "    IP 0x" << std::hex << ip_addr << " as bytes: ";
    for (auto b : ip_bytes) std::cout << std::dec << (int)b << ".";
    std::cout << "\b \n";

    return 0;
}
