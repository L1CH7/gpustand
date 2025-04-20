set(FFT_SOURCES
    GpuFourier.cxx
    AmFft.cxx
    FmFft.cxx
    FmFftSepNl.cxx
)

add_library(fft ${FFT_SOURCES})
