#include "nezalezhnyk/memory.hpp"
#include <stdexcept>

namespace nzk {

namespace {
// Знак-розширення значення довжиною bitsCount до 64-бітного int64_t,
// та сама логіка, що й у isa.cpp для декодування immediate.
int64_t signExtend(uint64_t value, int bitsCount) {
    uint64_t signBit = uint64_t{1} << (bitsCount - 1);
    return static_cast<int64_t>((value ^ signBit) - signBit);
}
}

bool isLoad(Mnemonic m) {
    switch (m) {
        case Mnemonic::LB: case Mnemonic::LH: case Mnemonic::LW: case Mnemonic::LD:
        case Mnemonic::LBU: case Mnemonic::LHU: case Mnemonic::LWU:
            return true;
        default:
            return false;
    }
}

bool isStore(Mnemonic m) {
    switch (m) {
        case Mnemonic::SB: case Mnemonic::SH: case Mnemonic::SW: case Mnemonic::SD:
            return true;
        default:
            return false;
    }
}

Memory::Memory(uint64_t sizeBytes) : data_(sizeBytes, 0) {}

uint8_t Memory::readByte(uint64_t addr) const {
    if (addr >= data_.size()) {
        throw std::out_of_range("Memory::readByte: адреса поза межами пам'яті");
    }
    return data_[addr];
}

void Memory::writeByte(uint64_t addr, uint8_t value) {
    if (addr >= data_.size()) {
        throw std::out_of_range("Memory::writeByte: адреса поза межами пам'яті");
    }
    data_[addr] = value;
}

uint64_t Memory::readN(uint64_t addr, int byteCount) const {
    uint64_t value = 0;
    for (int i = 0; i < byteCount; ++i) {
        value |= static_cast<uint64_t>(readByte(addr + static_cast<uint64_t>(i))) << (8 * i);
    }
    return value;
}

void Memory::writeN(uint64_t addr, uint64_t value, int byteCount) {
    for (int i = 0; i < byteCount; ++i) {
        writeByte(addr + static_cast<uint64_t>(i), static_cast<uint8_t>((value >> (8 * i)) & 0xFF));
    }
}

uint64_t Memory::load(Mnemonic loadOp, uint64_t addr) const {
    switch (loadOp) {
        case Mnemonic::LB:  return static_cast<uint64_t>(signExtend(readN(addr, 1), 8));
        case Mnemonic::LH:  return static_cast<uint64_t>(signExtend(readN(addr, 2), 16));
        case Mnemonic::LW:  return static_cast<uint64_t>(signExtend(readN(addr, 4), 32));
        case Mnemonic::LD:  return readN(addr, 8);
        case Mnemonic::LBU: return readN(addr, 1);
        case Mnemonic::LHU: return readN(addr, 2);
        case Mnemonic::LWU: return readN(addr, 4);
        default:
            throw std::invalid_argument("Memory::load: мнемоніка не є load-інструкцією");
    }
}

void Memory::store(Mnemonic storeOp, uint64_t addr, uint64_t value) {
    switch (storeOp) {
        case Mnemonic::SB: writeN(addr, value, 1); break;
        case Mnemonic::SH: writeN(addr, value, 2); break;
        case Mnemonic::SW: writeN(addr, value, 4); break;
        case Mnemonic::SD: writeN(addr, value, 8); break;
        default:
            throw std::invalid_argument("Memory::store: мнемоніка не є store-інструкцією");
    }
}

} // namespace nzk
