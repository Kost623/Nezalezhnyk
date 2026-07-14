#pragma once

#include <cstdint>
#include "nezalezhnyk/isa.hpp"

namespace nzk {

// Функції складання 32-бітного слова інструкції з полів.
// Дзеркальні до decode() — корисні для тестів і майбутнього асемблера.
namespace encode {

uint32_t rType(uint8_t opcode, uint8_t rd, uint8_t funct3,
                uint8_t rs1, uint8_t rs2, uint8_t funct7);

uint32_t iType(uint8_t opcode, uint8_t rd, uint8_t funct3,
                uint8_t rs1, int32_t imm12);

uint32_t sType(uint8_t opcode, uint8_t funct3,
                uint8_t rs1, uint8_t rs2, int32_t imm12);

uint32_t bType(uint8_t opcode, uint8_t funct3,
                uint8_t rs1, uint8_t rs2, int32_t imm12);

uint32_t jType(uint8_t opcode, uint8_t rd, int32_t imm20);

// Окремий хелпер під LUI: приймає вже зсунуту константу (imm << 12).
uint32_t luiType(uint8_t rd, uint32_t imm20);

} // namespace encode
} // namespace nzk
