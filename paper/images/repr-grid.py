#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np

AXIS_MAX=7

fig = plt.figure(figsize=(5,4.9))
ax = fig.add_subplot(projection='3d')

vols = {}
colors = {}
for h1, p1, o1, c1 in [(0, 0, 1, (0, 0, 0)), (1, 1, AXIS_MAX-1, (1, 0, 0))]:
    for h2, p2, o2, c2 in [(0, 0, 1, (0, 0, 0)), (1, 1, AXIS_MAX-1, (0, 1, 0))]:
        for h3, p3, o3, c3 in [(0, 0, 1, (0, 0, 0)), (1, 1, AXIS_MAX-1, (0, 0, 1))]:
            vols[h1, h2, h3] = [*(p1, p2, p3), *(o1, o2, o3)]
            colors[h1, h2, h3] = tuple(map(sum, zip(c1, c2, c3))) 

ax.bar3d(*vols[0, 0, 0], color=(*colors[0, 0, 0], 0.6), linewidth=2, edgecolor=(1, 1, 1, 0.3))
for key in [(1, 0, 0), (0, 1, 0), (0, 0, 1)]:
    ax.bar3d(*vols[key], color=(*colors[key], 0.45), linewidth=2, edgecolor=(1, 1, 1, 0.3))
for key in [(1, 1, 0), (0, 1, 1), (1, 0, 1)]:
    ax.bar3d(*vols[key], color=(*colors[key], 0.25), linewidth=2, edgecolor=(1, 1, 1, 0.3))
ax.bar3d(*vols[1, 1, 1], color=(*colors[1, 1, 1], 0.25), linewidth=2, edgecolor=(1, 1, 1, 0.3))

ax.set_aspect('equal', 'box')

ax.xaxis.line.set_color((0.9, 0.3, 0.3))
ax.yaxis.line.set_color((0.3, 0.9, 0.3))
ax.zaxis.line.set_color((0.3, 0.3, 0.9))
ax.xaxis.line.set_linewidth(5)
ax.yaxis.line.set_linewidth(5)
ax.zaxis.line.set_linewidth(5)
ax.set_xlabel("$m(b_1)$", fontsize=15)
ax.set_ylabel("$m(b_2)$", fontsize=15)
ax.set_zlabel("$m(b_3)$", fontsize=15)
ax.tick_params(axis='both', which='major', labelsize=12)

ax.view_init(135, 55, 180)
#ax.view_init(25, -115, 0)

'''
ax.set_axis_off()

ax.quiver3D(-0.2, -0.2, -0.2, AXIS_MAX+0.4, 0, 0, length=1, 
        arrow_length_ratio=0.1, colors=(1, 0.3, 0.3), linewidth=3)
ax.quiver3D(-0.2, -0.2, -0.2, 0, AXIS_MAX+0.4, 0, length=1, 
        arrow_length_ratio=0.1, colors=(0.3, 1, 0.3), linewidth=3)
ax.quiver3D(-0.2, -0.2, -0.2, 0, 0, AXIS_MAX+0.4, length=1, 
        arrow_length_ratio=0.1, colors=(0.3, 0.3, 1), linewidth=3)
ax.scatter(-0.2, -0.2, -0.2, color='k', s=30)
'''

plt.tight_layout()
plt.subplots_adjust(left=0.08, bottom=0.08)

#plt.show()
plt.savefig("repr-grid.pdf")
