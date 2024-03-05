#!/usr/bin/python3

import sys
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import math

# Load CSV.

data = {}

raw_data = []
with open(sys.argv[1], "r") as f:
    raw_data = f.readlines()

idx_to_label = []
labels = raw_data[0].strip().split(',')
for label in labels:
    idx_to_label += [label]
    data[label] = []

raw_data = raw_data[1:]

for raw_datum in raw_data:
    raw_datum_split = raw_datum.strip().split(',')
    for i, d in enumerate(raw_datum_split):
        if ('.' in d):
            data[idx_to_label[i]] += [float(d)]
        else:
            data[idx_to_label[i]] += [int(d)]

# Draw figures.

fig, ax = plt.subplots(ncols=3, figsize=(10, 3), layout="tight")

ax[0].set_box_aspect(0.75)
ax[1].set_box_aspect(0.75)
ax[2].set_box_aspect(0.75)

fig.subplots_adjust(bottom=0.27, wspace=0.4)

cov_unit_pow10 = 0 
_max_cov = data['Coverage'][-1]
while (True):
    _max_cov //= 1000 
    if (_max_cov <= 0): break
    cov_unit_pow10 += 3 

cov_unit_str = ""
if (cov_unit_pow10 > 0):
    cov_unit_str = " (x$10^{}$)".format(cov_unit_pow10)

time_unit = 1
_max_time = data['Time'][-1]
while (True):
    if (time_unit == 1):
        next_time_unit = 10
    elif (time_unit == 10):
        next_time_unit = 60
    elif (round(math.log(time_unit // 6, 10), 3).is_integer()):
        next_time_unit = time_unit * 2
    elif (round(math.log(time_unit // 12, 10), 3).is_integer()):
        next_time_unit = (time_unit // 2) * 5
    elif (round(math.log(time_unit // 30, 10), 3).is_integer()):
        next_time_unit = time_unit * 2
    _num_time_ticks = _max_time // next_time_unit
    if (_num_time_ticks <= 2): break
    time_unit = next_time_unit

time_fmt = ticker.FuncFormatter(lambda ms, x: int(ms / 60))
cov_fmt = ticker.FuncFormatter(lambda ms, x: int(ms / (10 ** cov_unit_pow10)))

ax[0].plot(data['Time'], data['Coverage'], color='C0')
ax[0].fill_between(data['Time'], data['Coverage'], color='C0', alpha=0.2)
ax[0].set_xlabel("Time (min)")
ax[0].set_ylabel("Logic State Coverage{}".format(cov_unit_str))
ax[0].set_xlim([data['Time'][0], data['Time'][-1]])
ax[0].set_ylim(bottom=0)
ax[0].xaxis.set_major_formatter(time_fmt)
ax[0].yaxis.set_major_formatter(cov_fmt)
ax[0].xaxis.set_major_locator(ticker.MultipleLocator(time_unit))

ax[1].plot(data['Time'], data['RateS(ins)'], color='C1', alpha=0.5)
ax[1].plot(data['Time'], data['RateS(avg)'], color='C1', linewidth=2)
ax[1].set_xlabel("Time (min)")
ax[1].set_ylabel("New Coverage (/sec)")
ax[1].set_xlim([data['Time'][0], data['Time'][-1]])
ax[1].legend(["Inst", "Avg"], loc="upper right")
ax[1].xaxis.set_major_formatter(time_fmt)
ax[1].xaxis.set_major_locator(ticker.MultipleLocator(time_unit))

ax[2].plot(data['Time'], data['RateE(ins)'], color='C2', alpha=0.5)
ax[2].plot(data['Time'], data['RateE(avg)'], color='C2', linewidth=2)
ax[2].set_xlabel("Time (min)")
ax[2].set_ylabel("New Coverage (/exec)")
ax[2].set_xlim([data['Time'][0], data['Time'][-1]])
ax[2].set_ylim([0, 100])
ax[2].legend(["Inst", "Avg"], loc="upper right")
ax[2].xaxis.set_major_formatter(time_fmt)
ax[2].yaxis.set_major_formatter('{x:.0f}%')
ax[2].xaxis.set_major_locator(ticker.MultipleLocator(time_unit))

# Save.

plt.show()
#plt.savefig('lscov.png')
