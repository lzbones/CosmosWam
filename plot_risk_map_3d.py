#!/usr/bin/env python3
import json
import os
import sys
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def main():
    json_path = "output/frame_result.json"
    if len(sys.argv) > 1:
        json_path = sys.argv[1]

    if not os.path.exists(json_path):
        print(f"Error: Risk map file not found at: {json_path}")
        sys.exit(1)

    print(f"Loading risk map from: {json_path}")
    with open(json_path, 'r') as f:
        data = json.load(f)

    grids = data.get("risk_grids", [])
    if not grids:
        print("Error: No risk grids found in JSON.")
        sys.exit(1)

    # Use the first frame (t = 0.0s) to visualize prediction horizon decay
    frame_idx = 0
    grid_frame = grids[frame_idx]
    t_s = grid_frame.get("t_s", 0.0)
    grid_size = grid_frame.get("grid_size", [200, 400])
    res = grid_frame.get("resolution_m", 0.25)
    flat_data = grid_frame.get("data", [])

    print(f"Generating equal-scale 3D surface plot for t = {t_s}s...")

    # Reconstruct the 2D grid matrix
    risk_matrix = np.array(flat_data).reshape(grid_size[1], grid_size[0])

    # Crop parameters matching the user's requirements:
    # X-axis (lateral road lanes: from -20m left to +20m right)
    lat_range = np.linspace(-20.0, 20.0, 160)
    # Y-axis (longitudinal travel direction: from -5m behind to +80m ahead)
    long_range = np.linspace(-5.0, 80.0, 200)

    # Create meshgrid: X is lateral, Y is longitudinal
    X_3d, Y_3d = np.meshgrid(lat_range, long_range)
    Z_3d = np.zeros_like(X_3d)

    for i in range(X_3d.shape[0]):
        for j in range(X_3d.shape[1]):
            lat_val = X_3d[i, j]
            long_val = Y_3d[i, j]

            # Convert relative meters to grid cell indices
            # Col corresponds to lateral offset (center at grid_size[0] // 2)
            # Row corresponds to longitudinal offset (center at 40 in C++)
            col_idx = int(lat_val / res + grid_size[0] // 2)
            row_idx = int(long_val / res + 40)

            # Clip to grid boundary limits
            col_idx = max(0, min(col_idx, grid_size[0] - 1))
            row_idx = max(0, min(row_idx, grid_size[1] - 1))

            Z_3d[i, j] = risk_matrix[row_idx, col_idx]

    # Set up figure
    fig = plt.figure(figsize=(12, 10), facecolor='white')
    ax = fig.add_subplot(111, projection='3d')
    ax.set_facecolor('white')

    # Plot the kinetic field surface mesh (using 'jet' colormap)
    surf = ax.plot_surface(X_3d, Y_3d, Z_3d, cmap='jet', edgecolor='none', 
                           linewidth=0, antialiased=True, alpha=0.95, rstride=1, cstride=1)

    # Labeling
    ax.set_xlabel("Lateral Offset x [m]", fontsize=11, fontweight='bold', labelpad=12)
    ax.set_ylabel("Longitudinal Offset y [m]", fontsize=11, fontweight='bold', labelpad=15)
    ax.set_zlabel("Field Strength |Ek|", fontsize=11, fontweight='bold', labelpad=5)
    
    # Adjust axes limits to highlight active lane region
    ax.set_xlim(-20, 20)
    ax.set_ylim(-5, 80)
    ax.set_zlim(0, 1.0)

    # ENFORCE CONSISTENT PHYSICAL ASPECT RATIO (1:1 scale for X and Y)
    # X span is 40m, Y span is 85m. We set box aspect to compress Z vertically for smoother view.
    ax.set_box_aspect((40, 85, 12))

    # Set view angle to look along the longitudinal direction from behind (South to North)
    # elev=22 degrees, azim=-95 degrees (slightly offset from -90 to see 3D profiles clearly)
    ax.view_init(elev=22, azim=-95)

    # Add colorbar
    cbar = fig.colorbar(surf, ax=ax, shrink=0.5, aspect=10, pad=0.08)
    cbar.set_label("Field Strength |Ek|", fontsize=10, fontweight='bold')

    plt.title(f"3D Vehicle Kinetic Energy Field |Ek| (t = {t_s:.1f}s)\n[Equal X-Y Scaling / X: Lateral, Y: Longitudinal]", 
              fontsize=13, fontweight='bold', pad=15)

    output_img = "output/risk_heatmap_3d.png"
    plt.savefig(output_img, dpi=150, bbox_inches="tight")
    plt.close()

    print(f"Successfully generated equal-scale 3D risk surface plot: {output_img}")

if __name__ == '__main__':
    main()
