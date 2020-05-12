#pragma once
#include "draw.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#ifndef EXEC_CONSTEXPR
#include <chrono>
#include <thread>
#endif

namespace CxChip8 {

constexpr uint8_t fontset[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0,
    0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80,
    0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
    0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80,
    0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80
};

template <typename T, int Size>
class CxStack {
private:
    std::array<T, Size> stack = {};
    int sp = -1;

public:
    constexpr CxStack() noexcept {};

    constexpr void push(uint16_t val) noexcept {
        stack[++sp] = val;
    }

    constexpr uint16_t pop() noexcept {
        return stack[sp--];
    }
};

struct Registers {
    uint8_t V[16];
    uint16_t I;
    uint8_t DT;
    uint8_t ST;
    uint16_t PC;
    uint8_t SP;
    uint8_t VF;
};


class Chip8 {
private:
    static constexpr uint16_t font_offset = 0x50;
    static constexpr uint16_t rom_offset = 0x200;
    static constexpr uint16_t display_width = 64;
    static constexpr uint16_t display_height = 32;
    static constexpr uint16_t ram_size = 0x1000;

    using Opcode = const uint16_t;
    using Address = const uint16_t;
    using Display = std::array<uint8_t, display_width * display_height>;
    using Stack = CxStack<uint16_t, 16>;

    uint8_t ram[ram_size]{};
    Stack stack{};
    Registers registers{};
    Display pixels = {};

    size_t cycle_cnt = 0;
    size_t cycle_limit = 0;

    uint16_t op = 0;

    bool interrupt = false;

public:
    template <const std::size_t N>
    constexpr Chip8(const uint8_t (&rom)[N], size_t cycle_limit = 0) noexcept;

    constexpr void run() noexcept;
    constexpr auto display() const noexcept;

private:
    constexpr void executeInst() noexcept;

    constexpr bool cycleLimitReached() const noexcept;

    constexpr Opcode fetchOpcode() const noexcept;
    constexpr Address addr() const noexcept;
    constexpr uint8_t xIndex() const noexcept;
    constexpr uint8_t yIndex() const noexcept;
    constexpr uint8_t& xValue() noexcept;
    constexpr uint8_t& yValue() noexcept;
    constexpr uint8_t kk() const noexcept;
    constexpr uint8_t rng() const noexcept;
    constexpr uint8_t nibble() const noexcept;

    constexpr void CLS() noexcept;
    constexpr void RET() noexcept;
    constexpr void JP_ADDR() noexcept;
    constexpr void CALL_ADDR() noexcept;
    constexpr void SE_X_KK() noexcept;
    constexpr void SNE_X_KK() noexcept;
    constexpr void SE_X_Y() noexcept;
    constexpr void LD_X_KK() noexcept;
    constexpr void ADD_X_KK() noexcept;
    constexpr void LD_X_Y() noexcept;
    constexpr void OR_X_Y() noexcept;
    constexpr void AND_X_Y() noexcept;
    constexpr void XOR_X_Y() noexcept;
    constexpr void ADD_X_Y() noexcept;
    constexpr void SUB_X_Y() noexcept;
    constexpr void SHR_X() noexcept;
    constexpr void SUBN_X_Y() noexcept;
    constexpr void SHL_X() noexcept;
    constexpr void SNE_X_Y() noexcept;
    constexpr void LD_I_ADDR() noexcept;
    constexpr void JP_0_ADDR() noexcept;
    constexpr void RND_X_KK() noexcept;
    constexpr void DRW_X_Y_NIB() noexcept;
    constexpr void SKP_X() noexcept;
    constexpr void SKNP_X() noexcept;
    constexpr void LD_X_DT() noexcept;
    constexpr void LD_X_K() noexcept;
    constexpr void LD_DT_X() noexcept;
    constexpr void LD_ST_X() noexcept;
    constexpr void ADD_I_X() noexcept;
    constexpr void LD_F_X() noexcept;
    constexpr void LD_B_X() noexcept;
    constexpr void LD_I_X() noexcept;
    constexpr void LD_X_I() noexcept;
};

template <const std::size_t N>
inline constexpr Chip8::Chip8(const uint8_t (&rom)[N], size_t cycle_limit) noexcept
: cycle_limit(cycle_limit) {
    // Load font
    for(int i = 0; i < sizeof(fontset); ++i)
        ram[font_offset + i] = fontset[i];

    // Load ROM
    for(int i = 0; i < N; ++i)
        ram[rom_offset + i] = rom[i];

    // Set program counter
    registers.PC = rom_offset;
}

inline constexpr auto Chip8::display() const noexcept {
    return pixels;
}

inline constexpr void Chip8::run() noexcept {
    op = fetchOpcode();
    while(op && !cycleLimitReached() && !interrupt) {
        registers.PC += 2;
        executeInst();

        if(registers.DT)
            registers.DT--;
        if(registers.ST)
            registers.ST--;

        cycle_cnt++;

        op = fetchOpcode();

#ifndef EXEC_CONSTEXPR
        printDisplay(display());
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
#endif
    }
}

inline constexpr Chip8::Opcode Chip8::fetchOpcode() const noexcept {
    auto op = ram[registers.PC] << 8;
    op |= ram[registers.PC + 1];
    return op;
}
inline constexpr Chip8::Address Chip8::addr() const noexcept {
    return op & 0x0FFF;
}

inline constexpr uint8_t Chip8::xIndex() const noexcept {
    return (op & 0x0F00) >> 8;
}

inline constexpr uint8_t Chip8::yIndex() const noexcept {
    return (op & 0x00F0) >> 4;
}

inline constexpr uint8_t& Chip8::xValue() noexcept {
    return registers.V[xIndex()];
}

inline constexpr uint8_t& Chip8::yValue() noexcept {
    return registers.V[yIndex()];
}

inline constexpr uint8_t Chip8::kk() const noexcept {
    return op & 0x00FF;
}

inline constexpr uint8_t Chip8::rng() const noexcept {
    return 0xFF; // TODO: Could be impl as constexpr
}

inline constexpr uint8_t Chip8::nibble() const noexcept {
    return op & 0x000F;
}

inline constexpr void Chip8::executeInst() noexcept {
    switch((op & 0xF000) >> 12) {
    case 0:
        switch(op) {
        case 0x00E0:
            CLS();
            break;
        case 0x00EE:
            RET();
            break;
        }
        break;
    case 1:
        JP_ADDR();
        break;
    case 2:
        CALL_ADDR();
        break;
    case 3:
        SE_X_KK();
        break;
    case 4:
        SNE_X_KK();
        break;
    case 5:
        SE_X_Y();
        break;
    case 6:
        LD_X_KK();
        break;
    case 7:
        ADD_X_KK();
        break;
    case 8:
        switch(nibble()) {
        case 0:
            LD_X_Y();
            break;
        case 1:
            OR_X_Y();
            break;
        case 2:
            AND_X_Y();
            break;
        case 3:
            XOR_X_Y();
            break;
        case 4:
            ADD_X_Y();
            break;
        case 5:
            SUB_X_Y();
            break;
        case 6:
            SHR_X();
            break;
        case 7:
            SUBN_X_Y();
            break;
        case 0x0E:
            SHL_X();
            break;
        }
        break;
    case 0x09:
        SNE_X_Y();
        break;
    case 0x0A:
        LD_I_ADDR();
        break;
    case 0x0B:
        JP_0_ADDR();
        break;
    case 0x0C:
        RND_X_KK();
        break;
    case 0x0D:
        DRW_X_Y_NIB();
        break;
    case 0x0E:
        switch(kk()) {
        case 0x9E:
            SKP_X();
            break;
        case 0xA1:
            SKNP_X();
            break;
        }
        break;
    case 0x0F:
        switch(kk()) {
        case 0x07:
            LD_X_DT();
            break;
        case 0x0A:
            LD_X_KK();
            break;
        case 0x15:
            LD_DT_X();
            break;
        case 0x18:
            LD_ST_X();
            break;
        case 0x1E:
            ADD_I_X();
            break;
        case 0x29:
            LD_F_X();
            break;
        case 0x33:
            LD_B_X();
            break;
        case 0x55:
            LD_I_X();
            break;
        case 0x65:
            LD_X_I();
            break;
        }
        break;
    }
}

inline constexpr bool Chip8::cycleLimitReached() const noexcept {
    if(cycle_limit == 0)
        return false;

    if(cycle_cnt >= cycle_limit)
        return true;
    return false;
}

inline constexpr void Chip8::CLS() noexcept {
    for(size_t i = 0; i < pixels.size(); i++)
        pixels[i] = 0;
}

inline constexpr void Chip8::RET() noexcept {
    registers.PC = stack.pop();
}

inline constexpr void Chip8::JP_ADDR() noexcept {
    registers.PC = addr();
}

inline constexpr void Chip8::CALL_ADDR() noexcept {
    stack.push(registers.PC + 2);
    registers.PC = addr();
}

inline constexpr void Chip8::SE_X_KK() noexcept {
    if(xValue() == kk())
        registers.PC += 2;
}

inline constexpr void Chip8::SNE_X_KK() noexcept {
    if(xValue() != kk())
        registers.PC += 2;
}

inline constexpr void Chip8::SE_X_Y() noexcept {
    if(xValue() == yValue())
        registers.PC += 2;
}

inline constexpr void Chip8::LD_X_KK() noexcept {
    xValue() = kk();
}

inline constexpr void Chip8::ADD_X_KK() noexcept {
    xValue() += kk();
}

inline constexpr void Chip8::LD_X_Y() noexcept {
    xValue() = yValue();
}

inline constexpr void Chip8::OR_X_Y() noexcept {
    xValue() |= yValue();
}

inline constexpr void Chip8::AND_X_Y() noexcept {
    xValue() &= yValue();
}

inline constexpr void Chip8::XOR_X_Y() noexcept {
    xValue() ^= yValue();
}

inline constexpr void Chip8::ADD_X_Y() noexcept {
    xValue() += yValue();
    if(yValue() > (0xFF - xValue()))
        registers.V[0x0F] = 1;
    else
        registers.V[0x0F] = 0;
}

inline constexpr void Chip8::SUB_X_Y() noexcept {
    registers.V[0x0F] = (xValue() > yValue()) ? 1 : 0;
    xValue() -= yValue();
}

inline constexpr void Chip8::SHR_X() noexcept {
    registers.V[0x0F] = xValue() & 0x01;
    xValue() >>= 1;
}

inline constexpr void Chip8::SUBN_X_Y() noexcept {
    registers.V[0x0F] = (yValue() > xValue()) ? 1 : 0;
    xValue() = yValue() - xValue();
}

inline constexpr void Chip8::SHL_X() noexcept {
    registers.V[0x0F] = (xValue() & 0x80) >> 7;
    xValue() <<= 1;
}

inline constexpr void Chip8::SNE_X_Y() noexcept {
    if(xValue() != yValue())
        registers.PC += 2;
}

inline constexpr void Chip8::LD_I_ADDR() noexcept {
    registers.I = addr();
}

inline constexpr void Chip8::JP_0_ADDR() noexcept {
    registers.PC = registers.V[0] + addr();
}

inline constexpr void Chip8::RND_X_KK() noexcept {
    xValue() = rng() & kk();
}

inline constexpr void Chip8::DRW_X_Y_NIB() noexcept {
    const uint8_t height = nibble();
    const uint8_t x_coord = xValue() % display_width;
    const uint8_t y_coord = yValue() % display_height;

    registers.V[0x0F] = 0;

    for(size_t row = 0; row < height; ++row) {
        uint8_t sprite_byte = ram[registers.I + row];
        for(size_t col = 0; col < 8; col++) {
            uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
            auto pixel = &pixels[(y_coord + row) * display_width + (x_coord + col)];
            if(sprite_pixel) {
                if(*pixel == 1)
                    registers.V[0x0F] = 1;
                *pixel ^= 1;
            }
        }
    }
}

inline constexpr void Chip8::SKP_X() noexcept {
    // Can't be supported at compile time
}

inline constexpr void Chip8::SKNP_X() noexcept {
    // Can't press keys in constexpr context, next inst always skipped
    registers.PC += 2;
}

inline constexpr void Chip8::LD_X_DT() noexcept {
    xValue() = registers.DT;
}

inline constexpr void Chip8::LD_X_K() noexcept {
    // No point in waiting for a key press in constexpr context, end exec instead
    interrupt = true;
}

inline constexpr void Chip8::LD_DT_X() noexcept {
    registers.DT = xValue();
}

inline constexpr void Chip8::LD_ST_X() noexcept {
    registers.ST = xValue();
}

inline constexpr void Chip8::ADD_I_X() noexcept {
    registers.I += xValue();
}

inline constexpr void Chip8::LD_F_X() noexcept {
    registers.I = font_offset + (5 * xValue());
}

inline constexpr void Chip8::LD_B_X() noexcept {
    auto v = xValue();
    ram[registers.I + 2] = v % 10;
    v /= 10;
    ram[registers.I + 1] = v % 10;
    v /= 10;
    ram[registers.I + 0] = v % 10;
}

inline constexpr void Chip8::LD_I_X() noexcept {
    for(uint8_t i = 0; i <= xIndex(); ++i)
        ram[registers.I + i] = registers.V[i];
    registers.I += xIndex() + 1;
}

inline constexpr void Chip8::LD_X_I() noexcept {
    for(uint8_t i = 0; i <= xIndex(); ++i)
        registers.V[i] = ram[registers.I + i];
    registers.I += xIndex() + 1;
}
} // namespace CxChip8