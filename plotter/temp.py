import json
import numpy as np
# import seaborn as sns
# import pandas as pd
# import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

def plot_complex(plt_t, fig_t, grd_t, filename, title, xlabel, ylabel):
    with open(filename, 'r') as file:
        data = json.load(file)
    real_parts = [item['real'] for item in data]
    imag_parts = [item['imag'] for item in data]

    amplitudes = np.sqrt(np.array(real_parts)**2 + np.array(imag_parts)**2)
    fig_ax = fig_t.add_subplot(grd_t)
    fig_ax.set_title(title)
    fig_ax.set_xlabel(xlabel)
    fig_ax.set_ylabel(ylabel)
    plt_t.plot(amplitudes)   

def plot_complex_r(plt_t, fig_t, grd_t, filename, title, xlabel, ylabel):
    with open(filename, 'r') as file:
        data = json.load(file)
    real_parts = [item['real'] for item in data]

    fig_ax = fig_t.add_subplot(grd_t)
    fig_ax.set_title(title)
    fig_ax.set_xlabel(xlabel)
    fig_ax.set_ylabel(ylabel)
    plt_t.plot(real_parts) 

def plot_complex_i(plt_t, fig_t, grd_t, filename, title, xlabel, ylabel):
    with open(filename, 'r') as file:
        data = json.load(file)
    imag_parts = [item['imag'] for item in data]

    fig_ax = fig_t.add_subplot(grd_t)
    fig_ax.set_title(title)
    fig_ax.set_xlabel(xlabel)
    fig_ax.set_ylabel(ylabel)
    plt_t.plot(imag_parts)   
    
    
# def plot_real(plt_t, fig_t, grd_t, filename, title, xlabel, ylabel):
#     with open(filename, 'r') as file:
#         data = json.load(file)
#     fig_ax = fig_t.add_subplot(grd_t)
#     fig_ax.set_title(title)
#     fig_ax.set_xlabel(xlabel)
#     fig_ax.set_ylabel(ylabel)
#     plt_t.plot(data)
    
    
fig = plt.figure(figsize=(30, 45), constrained_layout=True)
grd = gridspec.GridSpec(ncols=1, nrows=3, figure=fig)


plot_complex(plt, fig, grd[0,0],  '../data/outOfC418_02Polar1.json',              'Input_horz_ampl',       'Slice', 'Amplitude')
plot_complex_r(plt, fig, grd[1,0],  '../data/outOfC418_02Polar1.json',              'Input_horz_real',       'Slice', 'Amplitude')
plot_complex_i(plt, fig, grd[2,0],  '../data/outOfC418_02Polar1.json',              'Input_horz_imag',       'Slice', 'Amplitude')
# plot_complex(plt, fig, grd[2,0],  '../result/Gpu_tfpPreNormalization_vert.json',  'Result_vert',      'Slice', 'Amplitude')
# plot_complex(plt, fig, grd[1,0],  '../result/Gpu_tfpPreNormalization_horz.json',  'Result_horz',      'Slice', 'Amplitude')
# plot_real(   plt, fig, grd[2,0],  '../result/sins.json',                          'Sins result',      'Slice', 'Amplitude')
# plot_complex(plt, fig, grd[0,1],  '../result/event_1.json',                       'FFTPrep_F result', 'Slice', 'Amplitude')
# plot_complex(plt, fig, grd[1,1],  '../result/event_2.json',                       'FFT_FFT_F result', 'Slice', 'Amplitude')
# plot_complex(plt, fig, grd[2,1],  '../result/event_3.json',                       'FFTPost_F result', 'Slice', 'Amplitude')

# plt.tight_layout()
plt.show()
# plt.savefig('/home/lich/Downloads/GPU_Plot.png')