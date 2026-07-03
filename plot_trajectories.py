#!/usr/bin/env python3
import json
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

def main():
    json_path = "output/OutputVideo_trajectories.json"
    if len(sys.argv) > 1:
        json_path = sys.argv[1]

    if not os.path.exists(json_path):
        print(f"Error: Trajectories file not found at: {json_path}")
        sys.exit(1)

    print(f"Loading trajectories from: {json_path}")
    with open(json_path, 'r') as f:
        data = json.load(f)

    # Use a clean, modern style for the plot
    plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
    fig, ax = plt.subplots(figsize=(10, 10))

    # 1. Plot Ego Trajectory
    ego_traj = data.get("ego_trajectory", [])
    if ego_traj:
        ego_x = [pt["x"] for pt in ego_traj]
        ego_y = [pt["y"] for pt in ego_traj]
        ax.plot(ego_x, ego_y, color="#1e40af", label="Ego Vehicle (Self)", lw=3, zorder=5)
        ax.scatter(ego_x[0], ego_y[0], color="#22c55e", marker="o", s=120, label="Ego Start", edgecolors='black', zorder=6)
        ax.scatter(ego_x[-1], ego_y[-1], color="#ef4444", marker="X", s=120, label="Ego End", edgecolors='black', zorder=6)
        print(f"Plotted ego trajectory with {len(ego_x)} points.")

    # 2. Plot Environmental Vehicles
    env_vehicles = data.get("environmental_vehicles", {})
    colormap = plt.colormaps.get_cmap("tab10")
    for i, (veh_id, points) in enumerate(env_vehicles.items()):
        if not points:
            continue
        v_x = [pt["x"] for pt in points]
        v_y = [pt["y"] for pt in points]
        color = colormap(i % 10)
        ax.plot(v_x, v_y, linestyle="--", color=color, label=f"Vehicle ({veh_id})", lw=2, zorder=3)
        ax.scatter(v_x[0], v_y[0], color=color, marker="^", s=60, edgecolors='black', zorder=4)
        ax.scatter(v_x[-1], v_y[-1], color=color, marker="s", s=40, edgecolors='black', zorder=4)
        print(f"Plotted {veh_id} with {len(v_x)} points.")

    # Graph labeling & configuration
    ax.set_xlabel("East Offset X (meters)", fontsize=12, fontweight='bold', labelpad=8)
    ax.set_ylabel("North Offset Y (meters)", fontsize=12, fontweight='bold', labelpad=8)
    ax.set_title("CosmosWAM 2D Local ENU Trajectory Plot", fontsize=14, fontweight='bold', pad=15)
    
    # Legend settings
    ax.legend(loc="upper right", frameon=True, facecolor='white', framealpha=0.9, shadow=True, fontsize=10)
    
    # Equal scaling so 1 meter is the same size in both axes
    ax.set_aspect("equal", adjustable="datalim")

    # Save output plot
    output_img = "output/trajectory_plot.png"
    plt.savefig(output_img, dpi=150, bbox_inches="tight")
    plt.close()

    print(f"Successfully generated trajectory visualization plot: {output_img}")

if __name__ == '__main__':
    main()
