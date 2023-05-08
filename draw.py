import json
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Load JSON file
with open('pop_data.json') as f:
    data = json.load(f)

# Iterate over each set of results
for result_set in data:
    # Get the array of box positions and sizes
    positions = result_set['result']

    # Extract the x, y, and z coordinates and the box dimensions
    x = [p[0] for p in positions]
    y = [p[1] for p in positions]
    z = [p[2] for p in positions]
    w = [p[3] for p in positions]
    h = [p[4] for p in positions]
    d = [p[5] for p in positions]

    # Plot the pallet
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_xlim3d(0, max(x)+max(w))
    ax.set_ylim3d(0, max(y)+max(h))
    ax.set_zlim3d(0, max(z)+max(d))
    ax.view_init(azim=120, elev=30)

    # Plot each box
    for i in range(len(positions)):
        pos = positions[i]
        color = np.random.rand(3,)
        ax.bar3d(pos[0], pos[1], pos[2], pos[3], pos[4], pos[5], color=color, zsort='average')

    # Show the plot
    plt.show()
