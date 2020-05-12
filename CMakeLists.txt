cmake_minimum_required(VERSION 3.5)
project(ConstexprChip8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES 
   src/main.cpp
   src/Chip8.h
   src/draw.h
   src/roms.h
)

add_executable(
	ConstexprChip8
	${SOURCES}
)
