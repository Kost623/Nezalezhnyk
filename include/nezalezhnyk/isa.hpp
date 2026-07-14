#pragma once

#include <cstdint>
#include <string_view>

namespace nzk {

// Формати інструкцій, зафіксовані в ADR 002.
// SYSTEM — окремий тип для ECALL/EBREAK, які не мають операндів.
enum class Format {
    R,
    I,
    S,
    B,
    J,
    SYSTEM,
    UNKNOWN
};

// Opcode-константи. Старші 2 біти визначають групу (ADR 004),
// молодші 5 — конкретний під-код у межах групи.
namespace opcode {
    // Група 0: ALU (0-31)
    inline constexpr uint8_t ALU_R = 0b0000000; // регістр-регістр
    inline constexpr uint8_t ALU_I = 0b0000001; // з константою
    inline constexpr uint8_t LUI   = 0b0000010; // завантаження константи

    // Група 1: Пам'ять (32-63)
    inline constexpr uint8_t LOAD  = 0b0100000;
    inline constexpr uint8_t STORE = 0b0100001;

    // Група 2: Керування потоком (64-95)
    inline constexpr uint8_t BRANCH = 0b1000000;
    inline constexpr uint8_t JAL    = 0b1000001;
    inline constexpr uint8_t JALR   = 0b1000010;
    inline constexpr uint8_t SYSTEM = 0b1000011;

    // Група 3 (96-127) — зарезервована під майбутні розширення (ADR 004),
    // у базовому ISA свідомо не використовується.
}

// Повний список мнемонік базового набору з ADR 005.
enum class Mnemonic {
    // ALU, R-тип
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    // ALU, I-тип
    ADDI, SLLI, SLTI, SLTIU, XORI, SRLI, SRAI, ORI, ANDI,
    // Завантаження константи
    LUI_,
    // Пам'ять, load
    LB, LH, LW, LD, LBU, LHU, LWU,
    // Пам'ять, store
    SB, SH, SW, SD,
    // Умовні переходи
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    // Безумовні переходи
    JAL_, JALR_,
    // Системні
    ECALL, EBREAK,

    UNKNOWN
};

// Декодована інструкція — результат розбору 32-бітного слова.
struct Instruction {
    uint32_t raw = 0;
    Format format = Format::UNKNOWN;
    Mnemonic mnemonic = Mnemonic::UNKNOWN;
    uint8_t opcode = 0;
    uint8_t rd = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t funct3 = 0;
    uint8_t funct7 = 0;
    int64_t imm = 0; // вже знак-розширена константа, де застосовно
};

// Розбирає 32-бітне слово інструкції на структуру Instruction
// за правилами форматів з ADR 002 та opcode-таблицею з ADR 004-005.
Instruction decode(uint32_t raw);

// Людиночитна назва мнемоніки, напр. Mnemonic::ADD -> "add".
std::string_view mnemonicName(Mnemonic m);

// Людиночитна назва формату, напр. Format::R -> "R".
std::string_view formatName(Format f);

} // namespace nzk
