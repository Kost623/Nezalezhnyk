#pragma once

#include <cstdint>
#include "nezalezhnyk/isa.hpp"

namespace nzk {

// 10 базових операцій ALU. R- та I-тип мнемоніки (ADR 005) звужуються
// до цього самого набору — ADD і ADDI виконують ідентичну логіку,
// різниця лише в джерелі другого операнда (регістр vs immediate).
enum class AluOp {
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    UNSUPPORTED
};

// Визначає, якій ALU-операції відповідає мнемоніка.
// Повертає UNSUPPORTED для не-ALU інструкцій (load/store/branch/...).
AluOp aluOpFor(Mnemonic m);

// Виконує операцію над двома 64-бітними значеннями (ADR 002: 64-біт регістри).
// Зсуви маскують b до 6 біт (0-63) — коректний діапазон для 64-бітного регістра.
uint64_t executeAlu(AluOp op, uint64_t a, uint64_t b);

} // namespace nzk
