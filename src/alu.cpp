#include "nezalezhnyk/alu.hpp"

namespace nzk {

AluOp aluOpFor(Mnemonic m) {
    switch (m) {
        case Mnemonic::ADD:   case Mnemonic::ADDI:  return AluOp::ADD;
        case Mnemonic::SUB:                          return AluOp::SUB; // немає "subi" — addi з від'ємним imm замінює її
        case Mnemonic::SLL:   case Mnemonic::SLLI:   return AluOp::SLL;
        case Mnemonic::SLT:   case Mnemonic::SLTI:   return AluOp::SLT;
        case Mnemonic::SLTU:  case Mnemonic::SLTIU:  return AluOp::SLTU;
        case Mnemonic::XOR:   case Mnemonic::XORI:   return AluOp::XOR;
        case Mnemonic::SRL:   case Mnemonic::SRLI:   return AluOp::SRL;
        case Mnemonic::SRA:   case Mnemonic::SRAI:   return AluOp::SRA;
        case Mnemonic::OR:    case Mnemonic::ORI:    return AluOp::OR;
        case Mnemonic::AND:   case Mnemonic::ANDI:   return AluOp::AND;
        default: return AluOp::UNSUPPORTED;
    }
}

uint64_t executeAlu(AluOp op, uint64_t a, uint64_t b) {
    switch (op) {
        case AluOp::ADD:  return a + b;
        case AluOp::SUB:  return a - b;
        case AluOp::SLL:  return a << (b & 0x3F);
        case AluOp::SLT:  return (static_cast<int64_t>(a) < static_cast<int64_t>(b)) ? 1 : 0;
        case AluOp::SLTU: return (a < b) ? 1 : 0;
        case AluOp::XOR:  return a ^ b;
        case AluOp::SRL:  return a >> (b & 0x3F);
        case AluOp::SRA:  return static_cast<uint64_t>(static_cast<int64_t>(a) >> (b & 0x3F));
        case AluOp::OR:   return a | b;
        case AluOp::AND:  return a & b;
        default:          return 0;
    }
}

} // namespace nzk
