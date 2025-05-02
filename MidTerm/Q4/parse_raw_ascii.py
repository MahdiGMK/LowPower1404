def readRawFile(file_path):
    lines = []
    with open(file_path, "r") as f:
        for line in f:
            line = line.strip()
            # if not line: continue
            lines.append(line.split())
    res = {}
    res['title'] = lines[0][1]
    res['date'] = lines[1][1]
    res['command'] = ' '.join(lines[2][1:])
    res['plotname'] = ' '.join(lines[3][1:])
    res['flags'] = ' '.join(lines[4][1:])
    res['#var'] = int(lines[5][2])
    res['#point'] = int(lines[6][2])
    res['vars'] = (lines[8:(8+res['#var'])])
    idx = 8+res['#var'] + 1
    res['data'] = []
    for i in range(res['#point']):
        lst = [float(lines[idx][1])]
        idx += 1
        for j in range(1,res['#var']):
            lst.append(float(lines[idx][0]))
            idx += 1
        res['data'].append(lst)
    return res

import sys
rawfile = readRawFile(sys.argv[1])
total_energy = 0.0
last_t = 0.0
X = []
Y = []
for x in rawfile['data']:
    del_t = x[0] - last_t
    total_energy += del_t * x[1] * x[2]
    last_t = x[0]
    X.append(x[0])
    Y.append(total_energy)
    # Y.append(x[1] * x[2])
print(total_energy)
from matplotlib import pyplot as plt
plt.plot(X , Y)
plt.show()
