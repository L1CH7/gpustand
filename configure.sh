#!/bin/bash

fullpath="$(dirname $(readlink -f $0))"

# Укажите ccache как launcher для компилятора C++/C
if command -v ccache &> /dev/null; then 
    export CMAKE_CXX_COMPILER_LAUNCHER="ccache";
    export CMAKE_C_COMPILER_LAUNCHER="ccache"; 
fi

export CMAKE_BUILD_PARALLEL_LEVEL=#threads

CMAKE_CXX_COMPILER="x86_64-linux-gnu-g++"

# CMAKE_CXX_FLAGS="-pipe -O2 -funroll-loops -Wall -Wextra -Wformat=0 -Wmaybe-uninitialized"
# CMAKE_CXX_FLAGS="-pthread -pipe -funroll-loops"
# CMAKE_CXX_FLAGS="-O0 -ggdb -pthread -pipe -funroll-loops -fno-aggressive-loop-optimizations"
# CMAKE_CXX_FLAGS="-O2 -pthread -pipe -funroll-loops -fno-aggressive-loop-optimizations"
CMAKE_CXX_FLAGS="-O3 -pthread -pipe -funroll-loops -fno-aggressive-loop-optimizations"

BUILD_DIR="build"

# if [ -d "$BUILD_DIR" ]; then
#     rm -rf "$BUILD_DIR"
# fi

mkdir -p "$BUILD_DIR"
if command -v ninja &> /dev/null; then 
    CMAGE_GEN="-G Ninja"
else
    CMAGE_GEN=""
fi

cmake 	$CMAGE_GEN -B $BUILD_DIR -S "${fullpath}" 								\
		-D CMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" 							\
		-D CMAKE_CXX_COMPILER="$CMAKE_CXX_COMPILER" 					\
        -D CMAKE_BUILD_TYPE=debug									    \

if [ $? -eq 0 ]; then
    echo "CMake успешно запущен"
else
    echo "Ошибка запуска CMake"
    exit 1
fi

if command -v ninja &> /dev/null; then 
    ninja -C "$BUILD_DIR" -j12
else
    make -C "$BUILD_DIR" -j12
fi