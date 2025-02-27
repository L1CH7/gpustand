import json
import numpy as np
from pathlib import Path
# from Data import *

def dataComplexFromJsonFile(filepath: Path, polar: str):
    out = np.empty((0))
    if(not filepath.exists() or filepath.is_dir()):
        return out
    with filepath.open() as file:
        data = json.load(file)[polar]

    for ray in data:
        # print(data[ray])
        real_parts = [item[0] for item in data[ray]]
        # print(real_parts)
        imag_parts = [item[1] for item in data[ray]]
        c = np.array(real_parts, dtype=np.complex64)
        c.imag = imag_parts
        c = np.abs(c) # get amplitudes
        # c = c[:100]
        out = np.append(out, c)
    return out

def dataComplexFromJsonFileRAYPOLAR(filepath: Path, polar: str):
    out = np.empty((0))
    if(not filepath.exists() or filepath.is_dir()):
        return out
    with filepath.open() as file:
        data = json.load(file)

    for ray in data:
        # print(data[ray])
        # print(type(ray))
        # print(type(polar))
        # print(type(filepath))
        if polar not in ray: # skip if not our polar
            continue

        real_parts = [item[0] for item in data[ray]]
        # print(real_parts)
        imag_parts = [item[1] for item in data[ray]]
        c = np.array(real_parts, dtype=np.complex64)
        c.imag = imag_parts
        c = np.abs(c) # get amplitudes
        # c = c[:100]
        out = np.append(out, c)
    return out