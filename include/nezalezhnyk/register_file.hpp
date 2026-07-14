#pragma once

#include <array>
#include <cstdint>

namespace nzk {

// 32 регістри загального призначення, кожен 64-бітний (ADR 002).
// x0 за угодою (як у RISC-V) завжди повертає 0 і ігнорує запис —
// це дає "джерело нуля" без окремої інструкції чи спецрежиму.
class RegisterFile {
public:
    RegisterFile() { regs_.fill(0); }

    uint64_t read(uint8_t index) const {
        return (index == 0) ? 0 : regs_[index];
    }

    void write(uint8_t index, uint64_t value) {
        if (index != 0) {
            regs_[index] = value;
        }
    }

private:
    std::array<uint64_t, 32> regs_{};
};

} // namespace nzk
