#include <iostream>
#include <iomanip>
#include <vector>

#include "nezalezhnyk/isa.hpp"
#include "nezalezhnyk/encoder.hpp"
#include "nezalezhnyk/register_file.hpp"
#include "nezalezhnyk/alu.hpp"
#include "nezalezhnyk/memory.hpp"

namespace {

void printInstruction(const nzk::Instruction& ins) {
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << ins.raw
               << std::dec << std::setfill(' ')
               << "  " << std::left << std::setw(7) << nzk::mnemonicName(ins.mnemonic)
               << " [" << nzk::formatName(ins.format) << "]"
               << "  rd=x" << static_cast<int>(ins.rd)
               << " rs1=x" << static_cast<int>(ins.rs1)
               << " rs2=x" << static_cast<int>(ins.rs2)
               << " imm=" << ins.imm
               << '\n';
}

} // namespace

int main() {
    std::cout << "Nezalezhnyk — відкрита процесорна архітектура\n";
    std::cout << "Демонстрація: асемблюємо invitro-програму, декодуємо й виконуємо через ALU\n\n";

    using namespace nzk;

    std::vector<uint32_t> program = {
        encode::iType(opcode::ALU_I, /*rd=*/1, /*funct3=*/0b000, /*rs1=*/0, /*imm=*/5),          // addi x1, x0, 5
        encode::iType(opcode::ALU_I, /*rd=*/2, /*funct3=*/0b000, /*rs1=*/0, /*imm=*/7),          // addi x2, x0, 7
        encode::rType(opcode::ALU_R, /*rd=*/3, /*funct3=*/0b000, /*rs1=*/1, /*rs2=*/2, 0),        // add  x3, x1, x2
        encode::rType(opcode::ALU_R, /*rd=*/4, /*funct3=*/0b000, /*rs1=*/2, /*rs2=*/1, 0b0100000),// sub  x4, x2, x1
        encode::iType(opcode::ALU_I, /*rd=*/5, /*funct3=*/0b111, /*rs1=*/1, /*imm=*/3),          // andi x5, x1, 3
        encode::iType(opcode::ALU_I, /*rd=*/6, /*funct3=*/0b110, /*rs1=*/1, /*imm=*/2),          // ori  x6, x1, 2
        encode::iType(opcode::ALU_I, /*rd=*/7, /*funct3=*/0b100, /*rs1=*/1, /*imm=*/2),          // xori x7, x1, 2
        encode::iType(opcode::ALU_I, /*rd=*/8, /*funct3=*/0b001, /*rs1=*/1, /*imm=*/2),          // slli x8, x1, 2
        encode::iType(opcode::ALU_I, /*rd=*/9, /*funct3=*/0b101, /*rs1=*/2, /*imm=*/1),          // srli x9, x2, 1
        encode::iType(opcode::ALU_I, /*rd=*/10, /*funct3=*/0b010, /*rs1=*/1, /*imm=*/10),        // slti x10, x1, 10
        encode::sType(opcode::STORE, /*funct3=*/0b011, /*rs1=*/0, /*rs2=*/3, /*imm=*/0),          // sd   x3, 0(x0)
        encode::iType(opcode::LOAD, /*rd=*/11, /*funct3=*/0b011, /*rs1=*/0, /*imm=*/0),           // ld   x11, 0(x0)
        encode::iType(opcode::SYSTEM, /*rd=*/0, /*funct3=*/0, /*rs1=*/0, /*imm=*/0),              // ecall
    };

    RegisterFile regs;
    Memory mem(4096); // 4 КіБ демо-пам'яті

    for (uint32_t raw : program) {
        Instruction ins = decode(raw);
        printInstruction(ins);

        AluOp op = aluOpFor(ins.mnemonic);
        if (op != AluOp::UNSUPPORTED) {
            uint64_t a = regs.read(ins.rs1);
            uint64_t b = (ins.format == Format::R) ? regs.read(ins.rs2)
                                                       : static_cast<uint64_t>(ins.imm);
            regs.write(ins.rd, executeAlu(op, a, b));
        } else if (ins.mnemonic == Mnemonic::LUI_) {
            regs.write(ins.rd, static_cast<uint64_t>(ins.imm));
        } else if (isLoad(ins.mnemonic)) {
            uint64_t addr = regs.read(ins.rs1) + static_cast<uint64_t>(ins.imm);
            regs.write(ins.rd, mem.load(ins.mnemonic, addr));
        } else if (isStore(ins.mnemonic)) {
            uint64_t addr = regs.read(ins.rs1) + static_cast<uint64_t>(ins.imm);
            mem.store(ins.mnemonic, addr, regs.read(ins.rs2));
        }
        // branch/jump/system поки не виконуємо — наступний крок: керування потоком виконання
    }

    std::cout << "\nСтан регістрів після виконання:\n";
    struct Expected { int reg; uint64_t value; const char* note; };
    std::vector<Expected> expected = {
        {1, 5,  "addi"},
        {2, 7,  "addi"},
        {3, 12, "add x1+x2"},
        {4, 2,  "sub x2-x1"},
        {5, 1,  "andi x1&3"},
        {6, 7,  "ori x1|2"},
        {7, 7,  "xori x1^2"},
        {8, 20, "slli x1<<2"},
        {9, 3,  "srli x2>>1"},
        {10, 1, "slti x1<10"},
        {11, 12, "ld x3 назад з пам'яті"},
    };
    for (const auto& e : expected) {
        uint64_t actual = regs.read(static_cast<uint8_t>(e.reg));
        std::cout << "x" << e.reg << " = " << actual
                   << "  (" << e.note << ", очікується " << e.value << ")"
                   << (actual == e.value ? "  OK" : "  MISMATCH")
                   << '\n';
    }

    return 0;
}
