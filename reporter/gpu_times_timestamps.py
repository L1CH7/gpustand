from pathlib import Path
import matplotlib.pyplot as plt
from functools import cmp_to_key
from datetime import datetime
import numpy as np
import json

def compare_pairs(x, y):
    return x[2] - y[2]

def plot_sorted_pairs(time_pairs, p:Path):
    time_pairs = sorted(time_pairs, key=cmp_to_key(compare_pairs))
    # np_time_pairs = np.asarray(time_pairs)
    start_point = min(pair[0] for pair in time_pairs)

    total_starts = [pair[0]-start_point for pair in time_pairs]
    total_ends = [pair[1]-start_point for pair in time_pairs]
    fft_starts = [pair[2]-start_point for pair in time_pairs]
    fft_ends = [pair[3]-start_point for pair in time_pairs]

    # Создание индексов для оси Y
    indices = np.arange(len(time_pairs))

    # Создание фигуры и осей
    # fig, ax = plt.subplots()
    fig, ax = plt.subplots(figsize=(19, 13), constrained_layout=True)

    # Рисование линий для каждой пары
    for i in range(len(time_pairs)):
        ax.plot([total_starts[i], total_ends[i]], [indices[i], indices[i]], color='b', alpha=.7)
        ax.plot([fft_starts[i], fft_ends[i]], [indices[i], indices[i]], color='r')

    # Установка меток осей
    ax.set_xlabel(f'time(ms, total={max(total_ends)})')
    ax.set_ylabel(f'tests(total={len(time_pairs)})')
    ax.set_title('times_graph')

    # Показ графика
    plt.grid()
    plt.savefig(p/f'plot{datetime.now().time()}.png', bbox_inches='tight')
    plt.show()

if __name__ == "__main__":
    testcases = Path("testcases")
    # testcases = Path("testcases") / "report_2025-03-04_16:37:39"
    # testcases = Path("testcases") / "FM_copies"
    # testcases.mkdir(parents=True, exist_ok=True)
    # report_path = testcases / "report.txt"
    # report = report_path.open("w")
    # report.write(
    #     f"{'Test':50s}{'nl':5s}{'kgd':5s}{'kgrs':6s}{'Size':30s}{'Start GPU':30s}{'End GPU':30s}{'Total GPU time':30s}{'FFT GPU time':30s}\n"
    # )

    # time_pairs = np.empty((0))
    time_pairs = []
    for p in testcases.rglob("report*39/*.json"):
        with p.open() as report_file:
            report_json = json.load(report_file)
            nl = report_json["params"]["nl"]
            kgd = report_json["params"]["kgd"]
            kgrs = report_json["params"]["kgrs"]
            size = nl * kgd * kgrs

            times = report_json["time"]
            # report.write(
            #     f"{str(p):<50s}{nl:<5d}{kgd:<5d}{kgrs:<6d}{size:<30d}{times['total']['start']:<30.6f}{times['total']['end']:<30.6f}{times['total']['duration']:<30.6f}{times['fft']['duration']:<30.6f}\n"
            # )
            time_pairs.append((times['total']['start'],times['total']['end'],times['fft']['start'],times['fft']['end']))

    plot_sorted_pairs(time_pairs, testcases)
