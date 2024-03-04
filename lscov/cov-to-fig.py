import sys
import matplotlib.pyplot as plt
import numpy as np

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

fig, ax = plt.subplots(ncols=3, figsize=(10, 3))

#ax[0].set_box_aspect(0.75)
#ax[1].set_box_aspect(0.75)
#ax[2].set_box_aspect(0.75)

ax[0].plot(data['Time'], data['Coverage'], color='C0')
ax[0].fill_between(data['Time'], data['Coverage'], color='C0', alpha=0.2)
ax[0].set_xlabel("Time (sec)")
ax[0].set_ylabel("Logic State Coverage")
ax[0].set_xlim([data['Time'][0], data['Time'][-1]])
ax[0].set_ylim(bottom=0)

ax[1].plot(data['Time'], data['RateS(ins)'], color='C1', alpha=0.5)
ax[1].plot(data['Time'], data['RateS(avg)'], color='C1', linewidth=2)
ax[1].set_xlabel("Time (sec)")
ax[1].set_ylabel("New Coverage (/sec)")
ax[1].set_xlim([data['Time'][0], data['Time'][-1]])
ax[1].legend(["Instantaneous", "Average"], loc="upper right")

ax[2].plot(data['Time'], data['RateE(ins)'], color='C2', alpha=0.5)
ax[2].plot(data['Time'], data['RateE(avg)'], color='C2', linewidth=2)
ax[2].set_xlabel("Time (sec)")
ax[2].set_ylabel("New Coverage (/exec)")
ax[2].set_xlim([data['Time'][0], data['Time'][-1]])
ax[2].set_ylim([0, 100])
ax[2].legend(["Instantaneous", "Average"], loc="upper right")
ax[2].yaxis.set_major_formatter('{x:.0f}%')

fig.tight_layout()

# Save.

plt.show()
#plt.savefig('lscov.png')
