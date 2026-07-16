#pragma once

#include <cstdint>
#include <vector>
#include "nezalezhnyk/isa.hpp"

namespace nzk {

// Перевіряють, чи мнемоніка є load- чи store-інструкцією (ADR 005, група 1).
// Потрібні окремо від Format, бо Format::I ділять і load, і addi, і jalr.
bool isLoad(Mnemonic m);
bool isStore(Mnemonic m);

// Модуль пам'яті — плаский масив байтів, little-endian порядок байтів
// (як у RISC-V/x86 за замовчуванням; формальне рішення можна зафіксувати
// окремим ADR, якщо знадобиться змінити).
class Memory {
public:
    explicit Memory(uint64_t sizeBytes);

    // Читає значення за адресою відповідно до типу load-інструкції
    // (LB/LH/LW/LD — знакове розширення, LBU/LHU/LWU — беззнакове).
    uint64_t load(Mnemonic loadOp, uint64_t addr) const;

    // Записує значення за адресою відповідно до розміру store-інструкції.
    void store(Mnemonic storeOp, uint64_t addr, uint64_t value);

    // "Сирі" операції над 32-бітним словом, без прив'язки до конкретної
    // мнемоніки load/store — використовуються для завантаження коду програми
    // в пам'ять і вибірки інструкцій у циклі fetch-decode-execute.
    uint32_t fetchWord(uint64_t addr) const;
    void storeWord(uint64_t addr, uint32_t value);

private:
    std::vector<uint8_t> data_;

    uint8_t readByte(uint64_t addr) const;
    void writeByte(uint64_t addr, uint8_t value);

    uint64_t readN(uint64_t addr, int byteCount) const;
    void writeN(uint64_t addr, uint64_t value, int byteCount);
};

} // namespace nzk
