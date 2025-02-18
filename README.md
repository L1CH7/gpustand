## Installation
```bash
apt install ocl-icd opencl-clhpp opencl-headers opencl-nvidia clinfo opencl-rusticl-mesa opencl-clover-mesa 
```

## Compile and run (F5 in VSCode to debug)
```bash
./configure.sh && ./run.sh
```

## Configuring
1.  Open configure.sh
2.  Set these options in cmake command (ON/OFF):
    
    For time prifiling:

        -D ENABLE_TIME_PROFILING=ON
    For reading buffers info:
    
        -D ENABLE_DEBUG_COMPUTATIONS=ON    