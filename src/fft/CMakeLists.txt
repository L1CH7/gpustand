set(FFT_SOURCES
    GpuFourier.cpp
    AmFft.cpp
    FmFft.cpp
    FmFftSepNl.cpp
)

add_library(fft ${FFT_SOURCES})
