#include <cstdint>
#include <iostream>
#include <string>

#include "nezalezhnyk/alu.hpp"

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

} // namespace

int main() {
    using namespace nzk;

    check("ADD 5+7=12",           executeAlu(AluOp::ADD, 5, 7), 12);
    check("SUB 7-5=2",            executeAlu(AluOp::SUB, 7, 5), 2);
    check("AND 0b110&0b011",      executeAlu(AluOp::AND, 0b110, 0b011), 0b010);
    check("OR 0b110|0b011",       executeAlu(AluOp::OR,  0b110, 0b011), 0b111);
    check("XOR 0b110^0b011",      executeAlu(AluOp::XOR, 0b110, 0b011), 0b101);
    check("SLL 1<<4=16",          executeAlu(AluOp::SLL, 1, 4), 16);
    check("SRL 16>>4=1",          executeAlu(AluOp::SRL, 16, 4), 1);
    check("SLL маскує зсув (64&0x3F=0)", executeAlu(AluOp::SLL, 1, 64), 1);
    check("SLTU 3<5",             executeAlu(AluOp::SLTU, 3, 5), 1);
    check("SLTU 5<3",             executeAlu(AluOp::SLTU, 5, 3), 0);

    // -1 як бітовий патерн: 0xFFFF...FFFF. Знаково це -1 (менше за 1),
    // беззнаково — величезне число (більше за 1). ALU має розрізняти ці випадки.
    uint64_t minusOne = static_cast<uint64_t>(static_cast<int64_t>(-1));
    check("SLT -1<1 (знаково)",   executeAlu(AluOp::SLT, minusOne, 1), 1);
    check("SLTU -1<1 (беззнаково, -1 це велике число)",
          executeAlu(AluOp::SLTU, minusOne, 1), 0);

    // Арифметичний зсув вправо має зберігати знак (заповнення одиницями).
    uint64_t negEight = static_cast<uint64_t>(static_cast<int64_t>(-8));
    check("SRA -8>>1 = -4 (знак збережено)",
          executeAlu(AluOp::SRA, negEight, 1),
          static_cast<uint64_t>(static_cast<int64_t>(-4)));

    // R- та I-тип мнемоніки мають звужуватись до тієї самої ALU-операції.
    check("aluOpFor(ADD)==aluOpFor(ADDI)",
          static_cast<int>(aluOpFor(Mnemonic::ADD)) == static_cast<int>(aluOpFor(Mnemonic::ADDI)), 1);
    check("aluOpFor(LUI_) не підтримується ALU",
          static_cast<int>(aluOpFor(Mnemonic::LUI_)) == static_cast<int>(AluOp::UNSUPPORTED), 1);

    std::cout << '\n';
    if (failures == 0) {
        std::cout << "Усі тести пройдені успішно.\n";
    } else {
        std::cout << failures << " тест(и) провалено.\n";
    }
    return failures == 0 ? 0 : 1;
}
