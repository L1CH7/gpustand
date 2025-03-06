import matplotlib.pyplot as plt
import matplotlib.gridspec as gsp
from pathlib import Path

from plotter import *
from rw import *
    
# fig = plt.figure(figsize=(30, 45), constrained_layout=True)
# grd = gsp.GridSpec(ncols=1, nrows=1, figure=fig)

def plotAllAmplitudes__OLD(plt_t: plt, fig_t: plt.Figure, grd_t: GridSpec, test_p: Path, params_p: Path, polar: str):
    # testpath = Path('..') / 'testcases' / 'FM' / '000'
    # paramspath = testpath / 'in_args.json'
    with params_p.open() as f:
        params = json.load(f)
    I = nl = params['nl']

    # N = kgd = 100 #params['kgd']
    N = kgd = params['kgd']
    M = kgrs = params['kgrs']

    data = dataComplexFromJsonFile(test_p, str)

    # fig = plt.figure(figsize=(60, 90), constrained_layout=True)
    # grd = gsp.GridSpec(ncols=6, nrows=6, figure=fig)
    # grd = gsp.GridSpec(ncols=1, nrows=1, figure=fig)

    # data = np.abs(data)

    _x = np.arange(N)
    _y = np.arange(M)
    x, y = np.meshgrid(_x, _y)
    data = data.reshape(I, N, M)
    # for i in range(I):
    #     plot3d(plt, fig, grd[i//3,i%3], x, y, data[i].T, f"Result_horz[NL={i}]", 'kgd', 'kgrs', 'Amplitude')
    i=0
    plot3d(plt, fig, grd[0,0], x, y, data[i].T, f"Result_horz[NL={i}]", 'kgd', 'kgrs', 'Amplitude')
    # x, y = _x, _y
    # plot3dbars(plt, fig, grd[0,0], x, y, data[i].T, f"Result_horz[NL={i}]", 'kgd', 'kgrs', 'Amplitude')

    plt.show()

def plotAllAmplitudes_RES(plt_t: plt, fig_t: plt.Figure, grd_t: GridSpec, test_p: Path, params_p: Path, polar: str, i: int):
    with params_p.open() as f:
        params = json.load(f)
    I = nl = params['nl']
    N = kgd = params['kgd']
    M = kgrs = params['kgrs']

    data = dataComplexFromJsonFile(test_p, polar)

    _x = np.arange(N)
    _y = np.arange(M)
    x, y = np.meshgrid(_x, _y)
    data = data.reshape(I, N, M)

    if i==-1:
        i=0
    plot3d(plt_t, fig_t, grd_t, x, y, data[i].T, f"Result_horz[NL={i}]", 'kgd', 'kgrs', 'Amplitude')



def plotAllAmplitudes_RAYPOLAR(plt_t: plt, fig_t: plt.Figure, grd_t: GridSpec, test_p: Path, params_p: Path, polar: str, i: int, label: str):
    with params_p.open() as f:
        params = json.load(f)
    I = nl = params['nl']
    N = kgd = params['kgd']
    M = kgrs = params['kgrs']

    data = dataComplexFromJsonFileRAYPOLAR(test_p, polar)

    _x = np.arange(N)
    _y = np.arange(M)
    x, y = np.meshgrid(_x, _y)
    data = data.reshape(I, N, M)

    if i==-1:
        i=0
    plot3d_vertical(plt_t, fig_t, grd_t, x, y, data[i].T, label, 'kgd', 'kgrs', 'Amplitude')

def plotRealImag():
    fig = plt.figure(figsize=(60, 90), constrained_layout=True)
    grd = gsp.GridSpec(ncols=3, nrows=1, figure=fig)

    data = dataComplexFromJsonFile('../result/Gpu_tfpPreNormalization_horz.json')

    I = nl = 5
    N = kgd = 6
    M = kgrs = 7830
    _x = np.arange(N)
    _y = np.arange(M)
    x, y = np.meshgrid(_x, _y)

    i = 1 # get one NL

    data_t = data.reshape(I, N, M)
    data = np.abs(data_t)
    plot3d(plt, fig, grd[i//3,0], x, y, data[i].T, f"Amplitudes[NL={i}]", 'kgd', 'kgrs', 'Amplitude')

    data = np.real(data_t)
    plot3d(plt, fig, grd[i//3,1], x, y, data[i].T, f"Reals[NL={i}]", 'kgd', 'kgrs', 'Amplitude')

    data = np.imag(data_t)
    plot3d(plt, fig, grd[i//3,2], x, y, data[i].T, f"Imags[NL={i}]", 'kgd', 'kgrs', 'Amplitude')

    plt.show()

def plotInOut():
    in_data = dataComplexFromJsonFile('/home/lich/dev/gpustand/com_op_supp/src/gpu/RawData/FM_131_nl5_64x32/1/outOfC418_02Polar0.json')
    in_x = np.arange(len(in_data))
    out_data = dataComplexFromJsonFile('../result/Gpu_tfpPreNormalization_horz_FM.json')
    out_x = np.arange(len(out_data))
    fig = plt.figure(figsize=(60, 90), constrained_layout=True)
    grd = gsp.GridSpec(ncols=1, nrows=2, figure=fig)
    plot2d(plt, fig, grd[0,0], in_x, in_data, "Input", "Slice", "Amplitude")
    plot2d(plt, fig, grd[1,0], out_x, out_data, "Out", "Slice", "Amplitude")
    plt.show()

def plotWithParams(f_params, f_data):
    fig = plt.figure(figsize=(60, 90), constrained_layout=True)
    grd = gsp.GridSpec(ncols=3, nrows=2, figure=fig)

    data = dataComplexFromJsonFile('../result/Gpu_tfpPreNormalization_horz_FM.json')
    data = np.abs(data)

    I = nl = 5
    N = kgd = 64
    M = kgrs = 32
    _x = np.arange(N)
    _y = np.arange(M)
    x, y = np.meshgrid(_x, _y)
    data = data.reshape(I, N, M)
    # for i in range(I):
    #     plot3d(plt, fig, grd[i//3,i%3], x, y, data[i].T, f"Result_horz[NL={i}]", 'kgd', 'kgrs', 'Amplitude')
    i=0
    plot3d(plt, fig, grd[i//3,i%3], x, y, data[i].T, f"Result_horz[NL={i}]", 'kgd', 'kgrs', 'Amplitude')

    plt.show()

if __name__ == "__main__":

    fig = plt.figure(figsize=(60, 90), constrained_layout=True)
    grd = gsp.GridSpec(nrows=2, ncols=2, figure=fig)


    report_dir = Path('..') / 'reports' / 'report_2025-03-06_17:24:17'
    
    data_for_verify_dir = Path('..') / 'testcases' / 'FM' / '006'
    paramspath = data_for_verify_dir / 'in_args.json'
    data_for_verify_path = data_for_verify_dir / 'ftps.json'
    nl=0
    plotAllAmplitudes_RAYPOLAR(plt, fig, grd[0,0], data_for_verify_path, paramspath, 'Polar0', nl, 'for_verify')
    result_path = report_dir / 'data' / '006_result_polar0_2.json'
    plotAllAmplitudes_RAYPOLAR(plt, fig, grd[0,1], result_path, paramspath, 'Polar0', nl, 'result')

    
    # data_for_verify_dir = Path('..') / 'testcases' / 'FM' / '000'
    # paramspath = data_for_verify_dir / 'in_args.json'
    # data_for_verify_path = data_for_verify_dir / 'ftps.json'
    # nl=0
    # plotAllAmplitudes_RAYPOLAR(plt, fig, grd[1,0], data_for_verify_path, paramspath, 'Polar0', nl, 'for_verify')
    # result_path = report_dir / 'data' / '000_result_polar0.json'
    # plotAllAmplitudes_RAYPOLAR(plt, fig, grd[1,1], result_path, paramspath, 'Polar0', nl, 'result')

    plt.show()
