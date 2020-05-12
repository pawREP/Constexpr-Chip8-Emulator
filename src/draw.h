#pragma once

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <sstream>

//This print function is only intended to display the result of the constexpr emulation, it's not suitable as a continuous draw function.
template <typename Arr>
inline void printDisplay(const Arr& display) noexcept {
    _setmode(_fileno(stdout), _O_U16TEXT);

    const auto width = 64;
    const auto height = 32;

    std::wstringstream ss;
    ss << L"\x2554";
    for(size_t x = 0; x < width; x++)
        ss << L"\x2550";
    ss << L"\x2557\n";
    for(size_t y = 0; y < height; y++) {
        ss << L"\x2551";
        for(size_t x = 0; x < width; x++) {
            if(display[x + width * y]) {
                ss << L"\x2588";
            } else {
                ss << L" ";
            }
        }
        ss << L"\x2551\n";
    }
    ss << L"\x255A";
    for(size_t x = 0; x < width; x++)
        ss << L"\x2550";
    ss << L"\x255D\n";

    wprintf(L"%ws", ss.str().c_str());
}
