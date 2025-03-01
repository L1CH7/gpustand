from pathlib import Path
import json

if __name__ == "__main__":
    testcases = Path("testcases") / "FM_copies"
    testcases.mkdir(parents=True, exist_ok=True)
    report_path = testcases / "report.txt"
    report = report_path.open("w")
    report.write(
        f"{'Test':50s}{'nl':5s}{'kgd':5s}{'kgrs':6s}{'Size':30s}{'Start GPU':30s}{'End GPU':30s}{'Total GPU time':30s}{'FFT GPU time':30s}\n"
    )
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
