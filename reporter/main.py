from pathlib import Path
import matplotlib.pyplot as plt
from functools import cmp_to_key
import numpy as np
import json

def compare_pairs(x, y):
    return x[0] - y[0]

def plot_sorted_pairs(time_pairs):
    time_pairs = sorted(time_pairs, key=cmp_to_key(compare_pairs))
    total_starts = [pair[0] for pair in time_pairs]
    total_ends = [pair[1] for pair in time_pairs]
    fft_starts = [pair[2] for pair in time_pairs]
    fft_ends = [pair[3] for pair in time_pairs]

    # Создание индексов для оси Y
    indices = np.arange(len(time_pairs))

    # Создание фигуры и осей
    fig, ax = plt.subplots()

    # Рисование линий для каждой пары
    for i in range(len(time_pairs)):
        ax.plot([total_starts[i], total_ends[i]], [indices[i], indices[i]], color='b', alpha=.7)
        ax.plot([fft_starts[i], fft_ends[i]], [indices[i], indices[i]], color='r')

    # Установка меток осей
    ax.set_xlabel('time')
    ax.set_ylabel('test_idx')
    ax.set_title('times_graph')

    # Показ графика
    plt.show()

if __name__ == "__main__":
    testcases = Path("testcases") / "FM"
    testcases = Path("testcases") / "FM_copies"
    testcases.mkdir(parents=True, exist_ok=True)
    report_path = testcases / "report.txt"
    report = report_path.open("w")
    report.write(
        f"{'Test':50s}{'nl':5s}{'kgd':5s}{'kgrs':6s}{'Size':30s}{'Start GPU':30s}{'End GPU':30s}{'Total GPU time':30s}{'FFT GPU time':30s}\n"
    )

    # time_pairs = np.empty((0))
    time_pairs = []
    for p in testcases.rglob("*/result/time*.json"):
        params_path = p / "../../in_args.json"
        params_path = params_path.resolve()

        with params_path.open() as params_file:
            params = json.load(params_file)
            nl = params["nl"]
            kgd = params["kgd"]
            kgrs = params["kgrs"]
            size = nl * kgd * kgrs

        with p.open() as file:
            times = json.load(file)
            report.write(
                f"{str(p):<50s}{nl:<5d}{kgd:<5d}{kgrs:<6d}{size:<30d}{times['total']['start']:<30.6f}{times['total']['end']:<30.6f}{times['total']['duration']:<30.6f}{times['fft']['duration']:<30.6f}\n"
            )
            time_pairs.append((times['total']['start'],times['total']['end'],times['fft']['start'],times['fft']['end']))

    plot_sorted_pairs(time_pairs)
