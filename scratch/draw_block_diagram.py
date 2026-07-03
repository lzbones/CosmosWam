import matplotlib.pyplot as plt
import matplotlib.patches as patches

# Create a sleek, modern diagram using matplotlib
def draw_diagram():
    fig, ax = plt.subplots(figsize=(14, 8), facecolor='#121212')
    ax.set_facecolor('#121212')
    ax.set_xlim(0, 14)
    ax.set_ylim(0, 8)
    
    # Hide axes
    ax.axis('off')
    
    # Title
    ax.text(7, 7.3, "CosmosWAM Pipeline Architecture / 系统架构与数据流图", 
            color='#ffffff', fontsize=18, fontweight='bold', ha='center', va='center', fontname='Arial')
    
    # Helper to draw boxes
    def draw_box(x, y, w, h, title, subtitle, color, is_remote=True):
        # Glow effect (shadow box)
        shadow = patches.FancyBboxPatch((x-0.08, y-0.08), w+0.16, h+0.16, boxstyle="round,pad=0.1",
                                        facecolor=color, alpha=0.15, edgecolor='none')
        ax.add_patch(shadow)
        
        # Main box
        rect = patches.FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.1",
                                      facecolor='#1e1e1e', edgecolor=color, linewidth=2)
        ax.add_patch(rect)
        
        # Header text
        ax.text(x + w/2, y + h*0.7, title, color='#ffffff', fontsize=11, fontweight='bold', ha='center', va='center')
        # Subtitle text
        ax.text(x + w/2, y + h*0.3, subtitle, color='#b0b0b0', fontsize=9, ha='center', va='center')
        
        # Label Remote / Local
        label_color = '#bbf' if is_remote else '#f9f'
        label_text = "REMOTE / 云端" if is_remote else "LOCAL / 本地"
        ax.text(x + w/2, y + h*0.1, label_text, color=label_color, fontsize=7, fontweight='bold', ha='center', va='center')

    # Coordinates for boxes
    # Row 1 (Remote Phase)
    draw_box(0.5, 4.5, 3.2, 1.5, "Module 1: Uploader", "Image SCP Transfer\n图像上传与重命名", "#00bcd4", is_remote=True)
    draw_box(5.0, 4.5, 3.2, 1.5, "Module 2: FD Runner", "Cosmos-3 Video Gen\n视频生成及下载拷贝", "#ff4081", is_remote=True)
    draw_box(9.5, 4.5, 3.2, 1.5, "Module 3: LA Runner", "LocateAnything Bbox\n目标预测检测", "#ff9800", is_remote=True)
    
    # Row 2 (Local Phase)
    draw_box(0.5, 1.5, 2.6, 1.5, "Module 4: Downloader", "SCP Detections & Video\n检测结果与自车轨迹下载", "#4caf50", is_remote=False)
    draw_box(3.8, 1.5, 2.6, 1.5, "Module 5: Target Tracker", "Hungarian IoU Tracker\n匈牙利匹配与车头ROI过滤", "#9c27b0", is_remote=False)
    draw_box(7.1, 1.5, 2.6, 1.5, "Module 6: Coord Transformer", "IPM -> ENU -> WGS-84\n逆透视投影与坐标转换", "#3f51b5", is_remote=False)
    draw_box(10.4, 1.5, 2.6, 1.5, "Module 7: Risk Assessor", "Dynamic Potential Fields\n动态安全势能场评估", "#e91e63", is_remote=False)

    # Connections / Arrows
    def draw_arrow(x1, y1, x2, y2, text="", text_y_offset=0.25):
        ax.annotate("", xy=(x2, y2), xytext=(x1, y1),
                    arrowprops=dict(arrowstyle="->", color='#666666', lw=2, mutation_scale=15))
        if text:
            ax.text((x1+x2)/2, (y1+y2)/2 + text_y_offset, text, color='#888888', fontsize=8, ha='center', va='center')

    # Row 1 connections
    draw_arrow(3.85, 5.25, 4.85, 5.25, "SCP Upload")
    draw_arrow(8.35, 5.25, 9.35, 5.25, "Forecast Video")
    
    # Connecting Row 1 to Row 2
    # Arrow from Module 3 (right side) goes down and left to Module 4
    # We can draw a stepped arrow
    ax.annotate("", xy=(1.8, 3.2), xytext=(11.1, 4.3),
                arrowprops=dict(arrowstyle="->", color='#666666', lw=1.5, connectionstyle="bar,fraction=0.2", mutation_scale=12))
    ax.text(6.4, 3.9, "Download Detection Results & Video (SCP)", color='#888888', fontsize=8, ha='center', va='center')
    
    # Row 2 connections
    draw_arrow(3.25, 2.25, 3.65, 2.25)
    draw_arrow(6.55, 2.25, 6.95, 2.25)
    draw_arrow(9.85, 2.25, 10.25, 2.25)

    # Output to Plotting tools
    ax.annotate("", xy=(11.7, 0.4), xytext=(11.7, 1.3),
                arrowprops=dict(arrowstyle="->", color='#ffeb3b', lw=1.5, linestyle="dashed", mutation_scale=12))
    ax.text(11.7, 0.2, "Python Visualization Tools / 数据可视化绘图", color='#ffeb3b', fontsize=9, fontweight='bold', ha='center', va='center')

    plt.tight_layout()
    plt.savefig("docs/pipeline_block_diagram.png", dpi=150, facecolor='#121212')
    print("Successfully generated docs/pipeline_block_diagram.png")

if __name__ == "__main__":
    draw_diagram()
