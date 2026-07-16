#include <iostream>
#include <iomanip>
#include <vector>

#include "nezalezhnyk/isa.hpp"
#include "nezalezhnyk/encoder.hpp"
#include "nezalezhnyk/cpu.hpp"

using namespace nzk;

namespace {

void printCheck(const std::string& label, uint64_t actual, uint64_t expected) {
    std::cout << label << " = " << actual
               << "  (очікується " << expected << ")"
               << (actual == expected ? "  OK" : "  MISMATCH")
               << '\n';
}

// Демо 1: пряме виконання ALU + пам'ять (без переходів).
void demoStraightLine() {
    std::cout << "=== Демо 1: ALU + пам'ять (пряме виконання) ===\n";

    std::vector<uint32_t> program = {
        encode::iType(opcode::ALU_I, 1, 0b000, 0, 5),                 // addi x1, x0, 5
        encode::iType(opcode::ALU_I, 2, 0b000, 0, 7),                 // addi x2, x0, 7
        encode::rType(opcode::ALU_R, 3, 0b000, 1, 2, 0),               // add  x3, x1, x2
        encode::sType(opcode::STORE, 0b011, 0, 3, 0),                  // sd   x3, 0(x0)
        encode::iType(opcode::LOAD, 11, 0b011, 0, 0),                  // ld   x11, 0(x0)
        encode::iType(opcode::SYSTEM, 0, 0, 0, 0),                     // ecall
    };

    Memory mem(4096);
    Cpu cpu(mem);
    cpu.loadProgram(program);
    cpu.run();

    printCheck("x3 (5+7)", cpu.registers().read(3), 12);
    printCheck("x11 (ld назад з пам'яті)", cpu.registers().read(11), 12);
    std::cout << "Інструкцій виконано: " << cpu.instructionsExecuted() << "\n\n";
}

// Демо 2: цикл через умовний перехід — сума 1+2+3+4+5 = 15.
void demoBranchLoop() {
    std::cout << "=== Демо 2: цикл через branch (сума 1..5) ===\n";

    std::vector<uint32_t> program = {
        encode::iType(opcode::ALU_I, 1, 0b000, 0, 0),                  //  0: addi x1, x0, 0   (sum)
        encode::iType(opcode::ALU_I, 2, 0b000, 0, 1),                  //  4: addi x2, x0, 1   (i)
        encode::iType(opcode::ALU_I, 3, 0b000, 0, 6),                  //  8: addi x3, x0, 6   (bound)
        encode::rType(opcode::ALU_R, 1, 0b000, 1, 2, 0),                // 12: add  x1, x1, x2  <- loop:
        encode::iType(opcode::ALU_I, 2, 0b000, 2, 1),                  // 16: addi x2, x2, 1
        encode::bType(opcode::BRANCH, 0b001, 2, 3, -8),                 // 20: bne  x2, x3, loop
        encode::iType(opcode::SYSTEM, 0, 0, 0, 0),                     // 24: ecall
    };

    Memory mem(4096);
    Cpu cpu(mem);
    cpu.loadProgram(program);
    cpu.run();

    printCheck("x1 (сума 1..5)", cpu.registers().read(1), 15);
    printCheck("x2 (лічильник після циклу)", cpu.registers().read(2), 6);
    std::cout << "Інструкцій виконано: " << cpu.instructionsExecuted() << " (очікується 19)\n\n";
}

// Демо 3: jal/jalr — перевіряє збереження адреси повернення та непрямий перехід.
void demoJalJalr() {
    std::cout << "=== Демо 3: jal/jalr (виклик і непрямий перехід) ===\n";

    std::vector<uint32_t> program = {
        encode::jType(opcode::JAL, 1, 12),                              //  0: jal  x1, +12   -> jump to 12, x1=4
        encode::iType(opcode::ALU_I, 9, 0b000, 0, 111),                 //  4: addi x9, x0, 111  (виконається пізніше, через jalr)
        encode::iType(opcode::SYSTEM, 0, 0, 0, 0),                      //  8: ecall
        encode::iType(opcode::ALU_I, 3, 0b000, 0, 42),                  // 12: addi x3, x0, 42   (ціль jal)
        encode::iType(opcode::JALR, 0, 0b000, 1, 0),                     // 16: jalr x0, x1, 0    -> jump to x1=4
    };

    Memory mem(4096);
    Cpu cpu(mem);
    cpu.loadProgram(program);
    cpu.run();

    printCheck("x1 (адреса повернення від jal)", cpu.registers().read(1), 4);
    printCheck("x3 (встановлено в цілі jal)", cpu.registers().read(3), 42);
    printCheck("x9 (встановлено після jalr)", cpu.registers().read(9), 111);
    printCheck("x0 (завжди 0, навіть якщо rd=x0 в jalr)", cpu.registers().read(0), 0);
    std::cout << "Інструкцій виконано: " << cpu.instructionsExecuted() << " (очікується 5)\n\n";
}

} // namespace

int main() {
    std::cout << "Nezalezhnyk — відкрита процесорна архітектура\n";
    std::cout << "Демонстрація повного циклу fetch-decode-execute через клас Cpu\n\n";

    demoStraightLine();
    demoBranchLoop();
    demoJalJalr();

    return 0;
}
