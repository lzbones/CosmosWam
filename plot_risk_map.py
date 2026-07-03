#!/usr/bin/env python3
import json
import os
import sys
import numpy as np
import matplotlib.pyplot as plt

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

    # Use the first frame (t = 0.0s) as requested by the user to visualize prediction horizon decay
    frame_idx = 0
    grid_frame = grids[frame_idx]
    t_s = grid_frame.get("t_s", 0.0)
    grid_size = grid_frame.get("grid_size", [200, 400])
    res = grid_frame.get("resolution_m", 0.25)
    flat_data = grid_frame.get("data", [])

    print(f"Plotting risk map for frame at t = {t_s}s (Size: {grid_size[0]}x{grid_size[1]})")

    # Reshape grid data to 2D array (row-major format)
    risk_matrix = np.array(flat_data).reshape(grid_size[1], grid_size[0])

    # Set up matplotlib figure
    plt.style.use('dark_background')
    fig, ax = plt.subplots(figsize=(10, 10))

    # Grid boundaries relative to ego
    # X covers [-25, 25] relative to ego (centered at 100)
    # Y covers [-10, 90] relative to ego (centered at 40)
    extent = [
        -grid_size[0] * res / 2.0, grid_size[0] * res / 2.0,
        -40 * res, (grid_size[1] - 40) * res
    ]

    # Plot risk heatmap using the 'inferno' colormap (representing hot risk regions)
    im = ax.imshow(risk_matrix, extent=extent, origin='lower', cmap='inferno', vmin=0.0, vmax=1.0)
    cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cbar.set_label("Collision Risk Score (0.0 to 1.0)", fontsize=12, fontweight='bold', labelpad=12)

    # Draw Ego vehicle at the center of the BEV coordinate system (0, 0)
    # Ego vehicle dimensions: width = 1.8m, length = 4.8m
    ego_w = 1.8
    ego_l = 4.8
    # Determine color based on max stability risk
    stability_risk = data.get("stability_warning", {}).get("max_stability_risk", 0.0)
    ego_color = '#22c55e' # green
    if stability_risk >= 0.3:
        ego_color = '#eab308' # yellow
    if stability_risk >= 0.7:
        ego_color = '#ef4444' # red

    ego_rect = plt.Rectangle((-ego_w/2.0, -ego_l/2.0), ego_w, ego_l, 
                             edgecolor=ego_color, facecolor='none', lw=3, label=f'Ego Vehicle (Risk: {stability_risk:.2f})')
    ax.add_patch(ego_rect)

    # Draw range rings to help visualize distance offsets
    for radius in [10.0, 20.0, 30.0, 40.0]:
        circle = plt.Circle((0, 0), radius, color='white', fill=False, linestyle=':', alpha=0.3)
        ax.add_patch(circle)
        ax.text(0, radius + 0.5, f"{radius}m", color='white', fontsize=9, ha='center', alpha=0.5)

    # Labels and titles
    ax.set_xlabel("Relative East Offset (meters)", fontsize=12, fontweight='bold', labelpad=8)
    ax.set_ylabel("Relative North Offset (meters)", fontsize=12, fontweight='bold', labelpad=8)
    ax.set_title(f"CosmosWAM BEV Driving Risk Heatmap (t = {t_s:.1f}s)", fontsize=14, fontweight='bold', pad=15)
    ax.grid(True, linestyle=":", alpha=0.2, color="white")
    ax.legend(loc="upper right", facecolor='black', framealpha=0.7)

    # Ensure equal scaling
    ax.set_aspect("equal", adjustable="datalim")

    # Save output plot
    output_img = "output/risk_heatmap.png"
    plt.savefig(output_img, dpi=150, bbox_inches="tight")
    plt.close()

    print(f"Successfully generated risk heatmap plot: {output_img}")

if __name__ == '__main__':
    main()
