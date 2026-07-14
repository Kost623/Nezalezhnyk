#include "nezalezhnyk/encoder.hpp"

namespace nzk::encode {

namespace {
// Обрізає значення до потрібної кількості молодших біт —
// потрібно, щоб від'ємні imm коректно лягали у своє бітове поле.
constexpr uint32_t truncate(int32_t value, int bitsCount) {
    uint32_t mask = (bitsCount >= 32) ? 0xFFFFFFFFu : ((1u << bitsCount) - 1u);
    return static_cast<uint32_t>(value) & mask;
}
}

uint32_t rType(uint8_t opcode, uint8_t rd, uint8_t funct3,
                uint8_t rs1, uint8_t rs2, uint8_t funct7) {
    return (static_cast<uint32_t>(funct7) << 25) |
           (static_cast<uint32_t>(rs2)    << 20) |
           (static_cast<uint32_t>(rs1)    << 15) |
           (static_cast<uint32_t>(funct3) << 12) |
           (static_cast<uint32_t>(rd)     << 7)  |
            static_cast<uint32_t>(opcode);
}

uint32_t iType(uint8_t opcode, uint8_t rd, uint8_t funct3,
                uint8_t rs1, int32_t imm12) {
    return (truncate(imm12, 12)          << 20) |
           (static_cast<uint32_t>(rs1)    << 15) |
           (static_cast<uint32_t>(funct3) << 12) |
           (static_cast<uint32_t>(rd)     << 7)  |
            static_cast<uint32_t>(opcode);
}

uint32_t sType(uint8_t opcode, uint8_t funct3,
                uint8_t rs1, uint8_t rs2, int32_t imm12) {
    uint32_t imm = truncate(imm12, 12);
    uint32_t immHi = (imm >> 5) & 0x7F; // [11:5]
    uint32_t immLo = imm & 0x1F;        // [4:0]
    return (immHi << 25) |
           (static_cast<uint32_t>(rs2)    << 20) |
           (static_cast<uint32_t>(rs1)    << 15) |
           (static_cast<uint32_t>(funct3) << 12) |
           (immLo << 7) |
            static_cast<uint32_t>(opcode);
}

uint32_t bType(uint8_t opcode, uint8_t funct3,
                uint8_t rs1, uint8_t rs2, int32_t imm12) {
    // B-тип структурно ідентичний S-типу (ADR 002) — той самий розклад полів.
    return sType(opcode, funct3, rs1, rs2, imm12);
}

uint32_t jType(uint8_t opcode, uint8_t rd, int32_t imm20) {
    return (truncate(imm20, 20) << 12) |
           (static_cast<uint32_t>(rd) << 7) |
            static_cast<uint32_t>(opcode);
}

uint32_t luiType(uint8_t rd, uint32_t imm20) {
    return ((imm20 & 0xFFFFF) << 12) |
           (static_cast<uint32_t>(rd) << 7) |
            static_cast<uint32_t>(opcode::LUI);
}

} // namespace nzk::encode
