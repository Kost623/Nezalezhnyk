#include <iostream>
#include <iomanip>
#include <vector>

#include "nezalezhnyk/isa.hpp"
#include "nezalezhnyk/encoder.hpp"
#include "nezalezhnyk/register_file.hpp"

namespace {

// Друкує декодовану інструкцію в читабельному вигляді.
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
    std::cout << "Демонстрація: асемблюємо invitro-програму та декодуємо назад\n\n";

    using namespace nzk;

    // Невелика тестова "програма": x1 = 5; x2 = 7; x3 = x1 + x2; store x3 в пам'ять.
    std::vector<uint32_t> program = {
        encode::iType(opcode::ALU_I, /*rd=*/1, /*funct3=*/0b000, /*rs1=*/0, /*imm=*/5),   // addi x1, x0, 5
        encode::iType(opcode::ALU_I, /*rd=*/2, /*funct3=*/0b000, /*rs1=*/0, /*imm=*/7),   // addi x2, x0, 7
        encode::rType(opcode::ALU_R, /*rd=*/3, /*funct3=*/0b000, /*rs1=*/1, /*rs2=*/2, /*funct7=*/0), // add x3, x1, x2
        encode::sType(opcode::STORE, /*funct3=*/0b011, /*rs1=*/0, /*rs2=*/3, /*imm=*/0),  // sd x3, 0(x0)
        encode::iType(opcode::SYSTEM, /*rd=*/0, /*funct3=*/0, /*rs1=*/0, /*imm=*/0),      // ecall
    };

    RegisterFile regs;

    for (uint32_t raw : program) {
        Instruction ins = decode(raw);
        printInstruction(ins);

        // Мінімальне "виконання" — лише для демонстрації, поки без пам'яті/ALU-модуля.
        switch (ins.mnemonic) {
            case Mnemonic::ADDI:
                regs.write(ins.rd, regs.read(ins.rs1) + static_cast<uint64_t>(ins.imm));
                break;
            case Mnemonic::ADD:
                regs.write(ins.rd, regs.read(ins.rs1) + regs.read(ins.rs2));
                break;
            default:
                break; // store/ecall поки не виконуємо — це наступний крок
        }
    }

    std::cout << "\nСтан регістрів після виконання:\n";
    std::cout << "x1 = " << regs.read(1) << '\n';
    std::cout << "x2 = " << regs.read(2) << '\n';
    std::cout << "x3 = " << regs.read(3) << "  (очікується 12)\n";

    return 0;
}
