#!/bin/bash

fullpath="$(dirname $(readlink -f $0))"

CMAKE_CXX_COMPILER="x86_64-linux-gnu-g++"

# CMAKE_CXX_FLAGS="-pipe -O2 -funroll-loops -Wall -Wextra -Wformat=0 -Wmaybe-uninitialized"
# CMAKE_CXX_FLAGS="-pipe -funroll-loops"
CMAKE_CXX_FLAGS="-O3 -pthread -pipe -funroll-loops"

BUILD_DIR="build"

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

cmake 	-B "$BUILD_DIR" -S "${fullpath}" 								\
		-D CMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" 							\
		-D CMAKE_CXX_COMPILER="$CMAKE_CXX_COMPILER" 					\
        -D CMAKE_BUILD_TYPE=debug									    \
        # -D ENABLE_DEBUG_COMPUTATIONS=ON                                 \
        # -D ENABLE_TIME_PROFILING=ON                                     \


if [ $? -eq 0 ]; then
    echo "CMake успешно запущен"
else
    echo "Ошибка запуска CMake"
    exit 1
fi

make -C "$BUILD_DIR" -j12
