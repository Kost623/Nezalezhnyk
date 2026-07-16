#include <cstdint>
#include <functional>
#include <iostream>
#include <string>

#include "nezalezhnyk/memory.hpp"

namespace {

int failures = 0;

void check(const std::string& name, uint64_t actual, uint64_t expected) {
    if (actual != expected) {
        std::cout << "[FAIL] " << name << ": отримано " << actual
                   << ", очікувалось " << expected << '\n';
        ++failures;
    } else {
        std::cout << "[ OK ] " << name << '\n';
    }
}

void checkThrows(const std::string& name, const std::function<void()>& fn) {
    try {
        fn();
        std::cout << "[FAIL] " << name << ": винятку не було\n";
        ++failures;
    } catch (const std::out_of_range&) {
        std::cout << "[ OK ] " << name << '\n';
    }
}

} // namespace

int main() {
    using namespace nzk;

    Memory mem(64);

    // Round-trip для кожного розміру.
    mem.store(Mnemonic::SB, 0, 0xAB);
    check("SB/LBU round-trip", mem.load(Mnemonic::LBU, 0), 0xAB);

    mem.store(Mnemonic::SH, 8, 0x1234);
    check("SH/LHU round-trip", mem.load(Mnemonic::LHU, 8), 0x1234);

    mem.store(Mnemonic::SW, 16, 0xDEADBEEF);
    check("SW/LWU round-trip", mem.load(Mnemonic::LWU, 16), 0xDEADBEEF);

    mem.store(Mnemonic::SD, 24, 0x0123456789ABCDEF);
    check("SD/LD round-trip", mem.load(Mnemonic::LD, 24), 0x0123456789ABCDEF);

    // Знакове розширення: 0xFF як байт має читатись як -1 через LB,
    // але як 255 через LBU.
    mem.store(Mnemonic::SB, 32, 0xFF);
    check("LB знакове розширення 0xFF -> -1",
          mem.load(Mnemonic::LB, 32),
          static_cast<uint64_t>(static_cast<int64_t>(-1)));
    check("LBU беззнакове 0xFF -> 255", mem.load(Mnemonic::LBU, 32), 255);

    // Little-endian: молодший байт лежить за меншою адресою.
    mem.store(Mnemonic::SW, 40, 0x11223344);
    check("Little-endian: байт 0 = 0x44", mem.load(Mnemonic::LBU, 40), 0x44);
    check("Little-endian: байт 3 = 0x11", mem.load(Mnemonic::LBU, 43), 0x11);

    // Вихід за межі пам'яті має кидати виняток, а не тихо ламати дані.
    checkThrows("SB за межами пам'яті кидає виняток", [&]() {
        mem.store(Mnemonic::SB, 1000, 1);
    });
    checkThrows("LD за межами пам'яті кидає виняток", [&]() {
        mem.load(Mnemonic::LD, 60); // 60+8 > 64
    });

    std::cout << '\n';
    if (failures == 0) {
        std::cout << "Усі тести пройдені успішно.\n";
    } else {
        std::cout << failures << " тест(и) провалено.\n";
    }
    return failures == 0 ? 0 : 1;
}
