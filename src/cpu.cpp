#include "nezalezhnyk/cpu.hpp"
#include "nezalezhnyk/alu.hpp"

namespace nzk {

namespace {

bool isBranch(Mnemonic m) {
    switch (m) {
        case Mnemonic::BEQ: case Mnemonic::BNE: case Mnemonic::BLT:
        case Mnemonic::BGE: case Mnemonic::BLTU: case Mnemonic::BGEU:
            return true;
        default:
            return false;
    }
}

// Обчислює умову переходу для B-типу (ADR 005).
bool evaluateBranch(Mnemonic m, uint64_t a, uint64_t b) {
    switch (m) {
        case Mnemonic::BEQ:  return a == b;
        case Mnemonic::BNE:  return a != b;
        case Mnemonic::BLT:  return static_cast<int64_t>(a) < static_cast<int64_t>(b);
        case Mnemonic::BGE:  return static_cast<int64_t>(a) >= static_cast<int64_t>(b);
        case Mnemonic::BLTU: return a < b;
        case Mnemonic::BGEU: return a >= b;
        default:              return false;
    }
}

} // anonymous namespace

Cpu::Cpu(Memory& mem, uint64_t startPc) : mem_(mem), pc_(startPc) {}

void Cpu::loadProgram(const std::vector<uint32_t>& program, uint64_t startAddr) {
    uint64_t addr = startAddr;
    for (uint32_t word : program) {
        mem_.storeWord(addr, word);
        addr += 4;
    }
    pc_ = startAddr;
    halted_ = false;
    breakpointHit_ = false;
    instructionsExecuted_ = 0;
}

void Cpu::step() {
    if (halted_) {
        return;
    }

    uint32_t raw = mem_.fetchWord(pc_);
    Instruction ins = decode(raw);
    uint64_t nextPc = pc_ + 4; // типове значення — "наступна інструкція підряд"

    AluOp op = aluOpFor(ins.mnemonic);
    if (op != AluOp::UNSUPPORTED) {
        uint64_t a = regs_.read(ins.rs1);
        uint64_t b = (ins.format == Format::R) ? regs_.read(ins.rs2)
                                                   : static_cast<uint64_t>(ins.imm);
        regs_.write(ins.rd, executeAlu(op, a, b));
    } else if (ins.mnemonic == Mnemonic::LUI_) {
        regs_.write(ins.rd, static_cast<uint64_t>(ins.imm));
    } else if (isLoad(ins.mnemonic)) {
        uint64_t addr = regs_.read(ins.rs1) + static_cast<uint64_t>(ins.imm);
        regs_.write(ins.rd, mem_.load(ins.mnemonic, addr));
    } else if (isStore(ins.mnemonic)) {
        uint64_t addr = regs_.read(ins.rs1) + static_cast<uint64_t>(ins.imm);
        mem_.store(ins.mnemonic, addr, regs_.read(ins.rs2));
    } else if (isBranch(ins.mnemonic)) {
        uint64_t a = regs_.read(ins.rs1);
        uint64_t b = regs_.read(ins.rs2);
        if (evaluateBranch(ins.mnemonic, a, b)) {
            nextPc = pc_ + static_cast<uint64_t>(ins.imm);
        }
    } else if (ins.mnemonic == Mnemonic::JAL_) {
        regs_.write(ins.rd, pc_ + 4); // адреса повернення
        nextPc = pc_ + static_cast<uint64_t>(ins.imm);
    } else if (ins.mnemonic == Mnemonic::JALR_) {
        uint64_t target = (regs_.read(ins.rs1) + static_cast<uint64_t>(ins.imm))
                            & ~uint64_t{1}; // молодший біт скидається за угодою
        regs_.write(ins.rd, pc_ + 4);
        nextPc = target;
    } else if (ins.mnemonic == Mnemonic::ECALL) {
        // Спрощення поточного етапу: без повноцінної ОС/ABI трактуємо ecall
        // як контрольовану зупинку виконання. Повноцінна обробка системних
        // викликів — окремий майбутній крок.
        halted_ = true;
    } else if (ins.mnemonic == Mnemonic::EBREAK) {
        halted_ = true;
        breakpointHit_ = true;
    }
    // Mnemonic::UNKNOWN мовчки ігнорується на цьому етапі —
    // обробка помилок декодування буде окремим кроком.

    ++instructionsExecuted_;
    pc_ = nextPc;
}

void Cpu::run(uint64_t maxInstructions) {
    while (!halted_ && instructionsExecuted_ < maxInstructions) {
        step();
    }
}

} // namespace nzk
