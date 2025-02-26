#!/bin/zsh

export CUDA_PATH=/opt/cuda
export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH

(cd plotter && python main.py)
