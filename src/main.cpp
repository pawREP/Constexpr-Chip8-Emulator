#define EXEC_CONSTEXPR // DEBUG

#include "chip8.h"
#include "draw.h"
#include "roms.h"

template <const std::size_t N>
constexpr auto runVM(const uint8_t (&rom)[N], size_t cycle_limit = 0) noexcept {
    CxChip8::Chip8 vm(rom, cycle_limit);
    vm.run();
    return vm.display();
}

int main() {
#ifdef EXEC_CONSTEXPR
    constexpr auto display = runVM(roms::space_invaders, 500);
    printDisplay(display.data());
#else
    runVM(roms::space_invaders);
#endif

    std::wcin.ignore();
}
