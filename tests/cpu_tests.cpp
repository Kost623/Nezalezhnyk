#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "nezalezhnyk/cpu.hpp"
#include "nezalezhnyk/encoder.hpp"

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

void checkBool(const std::string& name, bool actual, bool expected) {
    check(name, actual ? 1 : 0, expected ? 1 : 0);
}

} // namespace

int main() {
    using namespace nzk;

    // 1. BEQ: перехід НЕ відбувається, коли регістри різні.
    {
        Memory mem(256);
        Cpu cpu(mem);
        std::vector<uint32_t> program = {
            encode::iType(opcode::ALU_I, 1, 0b000, 0, 1),  // addi x1, x0, 1
            encode::iType(opcode::ALU_I, 2, 0b000, 0, 2),  // addi x2, x0, 2
            encode::bType(opcode::BRANCH, 0b000, 1, 2, 100), // beq x1, x2, +100 (не спрацює)
            encode::iType(opcode::ALU_I, 3, 0b000, 0, 77), // addi x3, x0, 77 (має виконатись)
            encode::iType(opcode::SYSTEM, 0, 0, 0, 0),     // ecall
        };
        cpu.loadProgram(program);
        cpu.run();
        check("BEQ не спрацьовує при x1 != x2", cpu.registers().read(3), 77);
    }

    // 2. BEQ: перехід ВІДБУВАЄТЬСЯ, коли регістри рівні.
    {
        Memory mem(256);
        Cpu cpu(mem);
        std::vector<uint32_t> program = {
            encode::iType(opcode::ALU_I, 1, 0b000, 0, 5),   //  0: addi x1, x0, 5
            encode::iType(opcode::ALU_I, 2, 0b000, 0, 5),   //  4: addi x2, x0, 5
            encode::bType(opcode::BRANCH, 0b000, 1, 2, 8),   //  8: beq x1, x2, +8 -> ціль 16
            encode::iType(opcode::ALU_I, 3, 0b000, 0, 999),  // 12: addi x3, x0, 999 (має бути ПРОПУЩЕНО)
            encode::iType(opcode::SYSTEM, 0, 0, 0, 0),       // 16: ecall
        };
        cpu.loadProgram(program);
        cpu.run();
        check("BEQ спрацьовує при x1 == x2, x3 лишається 0", cpu.registers().read(3), 0);
    }

    // 3. JAL зберігає адресу повернення (pc+4) у rd.
    {
        Memory mem(256);
        Cpu cpu(mem);
        std::vector<uint32_t> program = {
            encode::jType(opcode::JAL, 5, 8),               // 0: jal x5, +8 -> ціль 8
            encode::iType(opcode::ALU_I, 6, 0b000, 0, 999),  // 4: (пропущено)
            encode::iType(opcode::SYSTEM, 0, 0, 0, 0),       // 8: ecall
        };
        cpu.loadProgram(program);
        cpu.run();
        check("JAL: x5 == pc+4 == 4", cpu.registers().read(5), 4);
        check("JAL: адреса 4 пропущена, x6 лишається 0", cpu.registers().read(6), 0);
    }

    // 4. JALR з rd=x0 не змінює x0 (жорстко зашитий нуль).
    {
        Memory mem(256);
        Cpu cpu(mem);
        std::vector<uint32_t> program = {
            encode::iType(opcode::ALU_I, 1, 0b000, 0, 8),    //  0: addi x1, x0, 8 (адреса переходу)
            encode::iType(opcode::JALR, 0, 0b000, 1, 0),      //  4: jalr x0, x1, 0 -> jump to 8
            encode::iType(opcode::ALU_I, 2, 0b000, 0, 555),   //  8: addi x2, x0, 555 (ціль jalr — виконується)
            encode::iType(opcode::SYSTEM, 0, 0, 0, 0),        // 12: ecall
        };
        cpu.loadProgram(program);
        cpu.run();
        checkBool("JALR: x0 лишається 0 навіть при rd=x0", cpu.registers().read(0) == 0, true);
        check("JALR: перехід приземлився на адресу 8, x2 виконався", cpu.registers().read(2), 555);
        check("pc після ecall на адресі 12", cpu.pc(), 16); // ecall не змінює nextPc, лише зупиняє подальші step()
    }

    // 5. ECALL зупиняє виконання (halted() == true), run() не заходить далі.
    {
        Memory mem(256);
        Cpu cpu(mem);
        std::vector<uint32_t> program = {
            encode::iType(opcode::SYSTEM, 0, 0, 0, 0),        // 0: ecall
            encode::iType(opcode::ALU_I, 1, 0b000, 0, 999),   // 4: (НЕ має виконатись)
        };
        cpu.loadProgram(program);
        cpu.run();
        checkBool("ECALL встановлює halted()", cpu.halted(), true);
        check("ECALL: інструкція після ecall не виконується", cpu.registers().read(1), 0);
        check("Виконано рівно 1 інструкцію", cpu.instructionsExecuted(), 1);
    }

    // 6. EBREAK зупиняє виконання й окремо позначається як breakpointHit().
    {
        Memory mem(256);
        Cpu cpu(mem);
        std::vector<uint32_t> program = {
            encode::iType(opcode::SYSTEM, 0, 0, 0, 1),        // 0: ebreak (imm=1 відрізняє від ecall)
        };
        cpu.loadProgram(program);
        cpu.run();
        checkBool("EBREAK встановлює halted()", cpu.halted(), true);
        checkBool("EBREAK встановлює breakpointHit()", cpu.breakpointHit(), true);
    }

    std::cout << '\n';
    if (failures == 0) {
        std::cout << "Усі тести пройдені успішно.\n";
    } else {
        std::cout << failures << " тест(и) провалено.\n";
    }
    return failures == 0 ? 0 : 1;
}
