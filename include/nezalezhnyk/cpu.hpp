#pragma once

#include <cstdint>
#include <vector>
#include "nezalezhnyk/isa.hpp"
#include "nezalezhnyk/register_file.hpp"
#include "nezalezhnyk/memory.hpp"

namespace nzk {

// Об'єднує регістровий файл і пам'ять у повний цикл fetch-decode-execute.
// PC (program counter) робить виконання нелінійним — з'являються
// умовні/безумовні переходи замість простого проходу по списку інструкцій.
class Cpu {
public:
    explicit Cpu(Memory& mem, uint64_t startPc = 0);

    // Завантажує машинний код у пам'ять за адресою startAddr
    // і встановлює PC на цю адресу.
    void loadProgram(const std::vector<uint32_t>& program, uint64_t startAddr = 0);

    // Виконує рівно одну інструкцію: fetch -> decode -> execute -> оновлення PC.
    void step();

    // Виконує step() у циклі, поки не спрацює halted() або не буде
    // вичерпано maxInstructions (запобіжник від нескінченних циклів).
    void run(uint64_t maxInstructions = 100000);

    bool halted() const { return halted_; }
    bool breakpointHit() const { return breakpointHit_; }
    uint64_t pc() const { return pc_; }
    uint64_t instructionsExecuted() const { return instructionsExecuted_; }

    RegisterFile& registers() { return regs_; }
    const RegisterFile& registers() const { return regs_; }

private:
    RegisterFile regs_;
    Memory& mem_;
    uint64_t pc_;
    bool halted_ = false;
    bool breakpointHit_ = false;
    uint64_t instructionsExecuted_ = 0;
};

} // namespace nzk
