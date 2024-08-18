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

fig, ax = plt.subplots(ncols=3, figsize=(10, 2.6), layout="tight")

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

# Decide time unit.

# FIXME: there must be a better algorithm for this kind of stuff.
time_unit_in_str = 'sec'
time_unit_in_fmt = 1
time_unit_in_sec = 1
_max_time = data['Time'][-1]

if (_max_time > 4*60*60):
    time_unit_in_str = 'hr'
    time_unit_in_fmt = 3600
elif (_max_time > 4*60):
    time_unit_in_str = 'min'
    time_unit_in_fmt = 60

while (True):
    if (time_unit_in_str == 'sec'):
        if (time_unit_in_sec == 1):
            next_time_unit_in_sec = 2
        elif (time_unit_in_sec == 2):
            next_time_unit_in_sec = 5
        elif (time_unit_in_sec == 5):
            next_time_unit_in_sec = 10 
        elif (round(math.log(time_unit_in_sec, 10), 3).is_integer()):
            next_time_unit_in_sec = time_unit_in_sec * 2
        elif (round(math.log(time_unit_in_sec // 2, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 2) * 5
        elif (round(math.log(time_unit_in_sec // 5, 10), 3).is_integer()):
            next_time_unit_in_sec = time_unit_in_sec * 2 
    elif (time_unit_in_str == 'min'):
        if (time_unit_in_sec == 1):
            next_time_unit_in_sec = 60
        elif (round(math.log(time_unit_in_sec // 6, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 2) * 3
        elif (round(math.log(time_unit_in_sec // 9, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 3) * 4 
        elif (round(math.log(time_unit_in_sec // 12, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 4) * 5
        elif (round(math.log(time_unit_in_sec // 15, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 5) * 6 
        elif (round(math.log(time_unit_in_sec // 18, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 3) * 4 
        elif (round(math.log(time_unit_in_sec // 24, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 4) * 5 
        elif (round(math.log(time_unit_in_sec // 30, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 2) * 3
        elif (round(math.log(time_unit_in_sec // 45, 10), 3).is_integer()):
            next_time_unit_in_sec = (time_unit_in_sec // 3) * 4 
    elif (time_unit_in_str == 'hr'):
        if (time_unit_in_sec == 1):
            next_time_unit_in_sec = 3600 
        else:
            next_time_unit_in_sec = ((time_unit_in_sec // 3600) + 1) * 3600
    _num_time_ticks = _max_time // next_time_unit_in_sec
    if (_num_time_ticks <= 3): break
    time_unit_in_sec = next_time_unit_in_sec

time_fmt = ticker.FuncFormatter(lambda ms, x: int(ms / time_unit_in_fmt))
cov_fmt = ticker.FuncFormatter(lambda ms, x: int(ms / (10 ** cov_unit_pow10)))

ax[0].plot(data['Time'], data['Coverage'], color='C0')
ax[0].fill_between(data['Time'], data['Coverage'], color='C0', alpha=0.2)
ax[0].set_xlabel("Time ({})".format(time_unit_in_str))
ax[0].set_ylabel("Logic State Coverage{}".format(cov_unit_str))
ax[0].set_xlim([data['Time'][0], data['Time'][-1]])
ax[0].set_ylim(bottom=0)
ax[0].xaxis.set_major_formatter(time_fmt)
ax[0].yaxis.set_major_formatter(cov_fmt)
ax[0].xaxis.set_major_locator(ticker.MultipleLocator(time_unit_in_sec))

ax[1].plot(data['Time'], data['RateS(ins)'], color='C1', alpha=0.5)
ax[1].plot(data['Time'], data['RateS(avg)'], color='C1', linewidth=2)
ax[1].set_xlabel("Time ({})".format(time_unit_in_str))
ax[1].set_ylabel("New Coverage (/sec)")
ax[1].set_xlim([data['Time'][0], data['Time'][-1]])
ax[1].set_ylim(bottom=0)
ax[1].legend(["Inst", "Avg"], loc="upper right")
ax[1].xaxis.set_major_formatter(time_fmt)
ax[1].xaxis.set_major_locator(ticker.MultipleLocator(time_unit_in_sec))

ax[2].plot(data['Time'], data['RateE(ins)'], color='C2', alpha=0.5)
ax[2].plot(data['Time'], data['RateE(avg)'], color='C2', linewidth=2)
ax[2].set_xlabel("Time ({})".format(time_unit_in_str))
ax[2].set_ylabel("New Coverage (/exec)")
ax[2].set_xlim([data['Time'][0], data['Time'][-1]])
ax[2].set_ylim([0, 100])
ax[2].legend(["Inst", "Avg"], loc="upper right")
ax[2].xaxis.set_major_formatter(time_fmt)
ax[2].yaxis.set_major_formatter('{x:.0f}%')
ax[2].xaxis.set_major_locator(ticker.MultipleLocator(time_unit_in_sec))

# Save.

#plt.savefig('lscov.pdf')
plt.show()
