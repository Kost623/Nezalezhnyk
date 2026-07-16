#include "nezalezhnyk/isa.hpp"

namespace nzk {

namespace {

// Витягує біти [hi:lo] з 32-бітного слова.
constexpr uint32_t bits(uint32_t value, int hi, int lo) {
    uint32_t width = static_cast<uint32_t>(hi - lo + 1);
    uint32_t mask = (width >= 32) ? 0xFFFFFFFFu : ((1u << width) - 1u);
    return (value >> lo) & mask;
}

// Знак-розширення значення довжиною bitsCount до 64-бітного int64_t.
// Критично: арифметика (XOR і віднімання) має відбуватись у 64-бітному
// домені — інакше переповнення "загортається" на 32 біти, і фінальний
// каст у int64_t дає невірний результат (додатне число замість від'ємного).
constexpr int64_t signExtend(uint32_t value, int bitsCount) {
    uint64_t v = value;
    uint64_t signBit = uint64_t{1} << (bitsCount - 1);
    return static_cast<int64_t>((v ^ signBit) - signBit);
}

// --- R-тип: funct3 + funct7 -> мнемоніка (ADR 005, група 0 під-код 0) ---
Mnemonic decodeAluR(uint8_t funct3, uint8_t funct7) {
    switch (funct3) {
        case 0b000: return (funct7 == 0b0100000) ? Mnemonic::SUB : Mnemonic::ADD;
        case 0b001: return Mnemonic::SLL;
        case 0b010: return Mnemonic::SLT;
        case 0b011: return Mnemonic::SLTU;
        case 0b100: return Mnemonic::XOR;
        case 0b101: return (funct7 == 0b0100000) ? Mnemonic::SRA : Mnemonic::SRL;
        case 0b110: return Mnemonic::OR;
        case 0b111: return Mnemonic::AND;
        default:    return Mnemonic::UNKNOWN;
    }
}

// --- I-тип ALU: funct3 (+ старший біт imm для SRLI/SRAI) (група 0 під-код 1) ---
Mnemonic decodeAluI(uint8_t funct3, uint32_t immRaw) {
    switch (funct3) {
        case 0b000: return Mnemonic::ADDI;
        case 0b001: return Mnemonic::SLLI;
        case 0b010: return Mnemonic::SLTI;
        case 0b011: return Mnemonic::SLTIU;
        case 0b100: return Mnemonic::XORI;
        case 0b101: return (bits(immRaw, 11, 11) == 1) ? Mnemonic::SRAI : Mnemonic::SRLI;
        case 0b110: return Mnemonic::ORI;
        case 0b111: return Mnemonic::ANDI;
        default:    return Mnemonic::UNKNOWN;
    }
}

Mnemonic decodeLoad(uint8_t funct3) {
    switch (funct3) {
        case 0b000: return Mnemonic::LB;
        case 0b001: return Mnemonic::LH;
        case 0b010: return Mnemonic::LW;
        case 0b011: return Mnemonic::LD;
        case 0b100: return Mnemonic::LBU;
        case 0b101: return Mnemonic::LHU;
        case 0b110: return Mnemonic::LWU;
        default:    return Mnemonic::UNKNOWN;
    }
}

Mnemonic decodeStore(uint8_t funct3) {
    switch (funct3) {
        case 0b000: return Mnemonic::SB;
        case 0b001: return Mnemonic::SH;
        case 0b010: return Mnemonic::SW;
        case 0b011: return Mnemonic::SD;
        default:    return Mnemonic::UNKNOWN;
    }
}

Mnemonic decodeBranch(uint8_t funct3) {
    switch (funct3) {
        case 0b000: return Mnemonic::BEQ;
        case 0b001: return Mnemonic::BNE;
        case 0b100: return Mnemonic::BLT;
        case 0b101: return Mnemonic::BGE;
        case 0b110: return Mnemonic::BLTU;
        case 0b111: return Mnemonic::BGEU;
        default:    return Mnemonic::UNKNOWN;
    }
}

} // anonymous namespace

Instruction decode(uint32_t raw) {
    Instruction ins;
    ins.raw = raw;
    ins.opcode = static_cast<uint8_t>(bits(raw, 6, 0));

    switch (ins.opcode) {
        case opcode::ALU_R: {
            ins.format = Format::R;
            ins.rd     = static_cast<uint8_t>(bits(raw, 11, 7));
            ins.funct3 = static_cast<uint8_t>(bits(raw, 14, 12));
            ins.rs1    = static_cast<uint8_t>(bits(raw, 19, 15));
            ins.rs2    = static_cast<uint8_t>(bits(raw, 24, 20));
            ins.funct7 = static_cast<uint8_t>(bits(raw, 31, 25));
            ins.mnemonic = decodeAluR(ins.funct3, ins.funct7);
            break;
        }
        case opcode::ALU_I: {
            ins.format = Format::I;
            ins.rd     = static_cast<uint8_t>(bits(raw, 11, 7));
            ins.funct3 = static_cast<uint8_t>(bits(raw, 14, 12));
            ins.rs1    = static_cast<uint8_t>(bits(raw, 19, 15));
            uint32_t immRaw = bits(raw, 31, 20);
            ins.imm = signExtend(immRaw, 12);
            ins.mnemonic = decodeAluI(ins.funct3, immRaw);
            break;
        }
        case opcode::LUI: {
            ins.format = Format::J; // той самий бітовий розклад, що й J-тип
            ins.rd  = static_cast<uint8_t>(bits(raw, 11, 7));
            ins.imm = static_cast<int64_t>(bits(raw, 31, 12)) << 12;
            ins.mnemonic = Mnemonic::LUI_;
            break;
        }
        case opcode::LOAD: {
            ins.format = Format::I;
            ins.rd     = static_cast<uint8_t>(bits(raw, 11, 7));
            ins.funct3 = static_cast<uint8_t>(bits(raw, 14, 12));
            ins.rs1    = static_cast<uint8_t>(bits(raw, 19, 15));
            ins.imm = signExtend(bits(raw, 31, 20), 12);
            ins.mnemonic = decodeLoad(ins.funct3);
            break;
        }
        case opcode::STORE: {
            ins.format = Format::S;
            ins.funct3 = static_cast<uint8_t>(bits(raw, 14, 12));
            ins.rs1    = static_cast<uint8_t>(bits(raw, 19, 15));
            ins.rs2    = static_cast<uint8_t>(bits(raw, 24, 20));
            uint32_t immRaw = (bits(raw, 31, 25) << 5) | bits(raw, 11, 7);
            ins.imm = signExtend(immRaw, 12);
            ins.mnemonic = decodeStore(ins.funct3);
            break;
        }
        case opcode::BRANCH: {
            ins.format = Format::B;
            ins.funct3 = static_cast<uint8_t>(bits(raw, 14, 12));
            ins.rs1    = static_cast<uint8_t>(bits(raw, 19, 15));
            ins.rs2    = static_cast<uint8_t>(bits(raw, 24, 20));
            uint32_t immRaw = (bits(raw, 31, 25) << 5) | bits(raw, 11, 7);
            ins.imm = signExtend(immRaw, 12);
            ins.mnemonic = decodeBranch(ins.funct3);
            break;
        }
        case opcode::JAL: {
            ins.format = Format::J;
            ins.rd  = static_cast<uint8_t>(bits(raw, 11, 7));
            ins.imm = signExtend(bits(raw, 31, 12), 20);
            ins.mnemonic = Mnemonic::JAL_;
            break;
        }
        case opcode::JALR: {
            ins.format = Format::I;
            ins.rd     = static_cast<uint8_t>(bits(raw, 11, 7));
            ins.funct3 = static_cast<uint8_t>(bits(raw, 14, 12));
            ins.rs1    = static_cast<uint8_t>(bits(raw, 19, 15));
            ins.imm = signExtend(bits(raw, 31, 20), 12);
            ins.mnemonic = Mnemonic::JALR_;
            break;
        }
        case opcode::SYSTEM: {
            ins.format = Format::SYSTEM;
            ins.imm = signExtend(bits(raw, 31, 20), 12); // 0 = ECALL, 1 = EBREAK
            ins.mnemonic = (ins.imm == 0) ? Mnemonic::ECALL : Mnemonic::EBREAK;
            break;
        }
        default:
            ins.format = Format::UNKNOWN;
            ins.mnemonic = Mnemonic::UNKNOWN;
            break;
    }
    return ins;
}

std::string_view mnemonicName(Mnemonic m) {
    switch (m) {
        case Mnemonic::ADD:    return "add";
        case Mnemonic::SUB:    return "sub";
        case Mnemonic::SLL:    return "sll";
        case Mnemonic::SLT:    return "slt";
        case Mnemonic::SLTU:   return "sltu";
        case Mnemonic::XOR:    return "xor";
        case Mnemonic::SRL:    return "srl";
        case Mnemonic::SRA:    return "sra";
        case Mnemonic::OR:     return "or";
        case Mnemonic::AND:    return "and";
        case Mnemonic::ADDI:   return "addi";
        case Mnemonic::SLLI:   return "slli";
        case Mnemonic::SLTI:   return "slti";
        case Mnemonic::SLTIU:  return "sltiu";
        case Mnemonic::XORI:   return "xori";
        case Mnemonic::SRLI:   return "srli";
        case Mnemonic::SRAI:   return "srai";
        case Mnemonic::ORI:    return "ori";
        case Mnemonic::ANDI:   return "andi";
        case Mnemonic::LUI_:   return "lui";
        case Mnemonic::LB:     return "lb";
        case Mnemonic::LH:     return "lh";
        case Mnemonic::LW:     return "lw";
        case Mnemonic::LD:     return "ld";
        case Mnemonic::LBU:    return "lbu";
        case Mnemonic::LHU:    return "lhu";
        case Mnemonic::LWU:    return "lwu";
        case Mnemonic::SB:     return "sb";
        case Mnemonic::SH:     return "sh";
        case Mnemonic::SW:     return "sw";
        case Mnemonic::SD:     return "sd";
        case Mnemonic::BEQ:    return "beq";
        case Mnemonic::BNE:    return "bne";
        case Mnemonic::BLT:    return "blt";
        case Mnemonic::BGE:    return "bge";
        case Mnemonic::BLTU:   return "bltu";
        case Mnemonic::BGEU:   return "bgeu";
        case Mnemonic::JAL_:   return "jal";
        case Mnemonic::JALR_:  return "jalr";
        case Mnemonic::ECALL:  return "ecall";
        case Mnemonic::EBREAK: return "ebreak";
        default:                return "unknown";
    }
}

std::string_view formatName(Format f) {
    switch (f) {
        case Format::R:      return "R";
        case Format::I:      return "I";
        case Format::S:      return "S";
        case Format::B:      return "B";
        case Format::J:      return "J";
        case Format::SYSTEM: return "SYSTEM";
        default:             return "UNKNOWN";
    }
}

} // namespace nzk
