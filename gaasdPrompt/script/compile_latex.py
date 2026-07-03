#!/usr/bin/env python3
"""
LaTeX to PDF 编译与自动修正、清理脚本
使用方法：python <PromptDir>/script/compile_latex.py [tex_file.tex] [output_dir]

# 根据 C/C++ 源代码路径自动映射到 doc/ 目录：
#   - [ProjectName]/src/func/[FunctionName].cpp  →  [ProjectName]/doc/[FunctionName]/[FunctionName].tex
#   - [ProjectName]/src/mbd/[SubModuleName].cpp  →  [ProjectName]/doc/[SubModuleName]/[SubModuleName].tex
#   - [ProjectName]/include/func/[FunctionName].hpp  →  [ProjectName]/doc/[FunctionName]/[FunctionName].tex
"""

import subprocess
import sys
import os
from pathlib import Path


def src_to_doc_path(src_path: str, doc_root: str = "doc") -> str:
    """
    根据组件工程物理隔离规范，将 [ProjectName]/src/func/[FunctionName].cpp
    直接映射到 [ProjectName]/doc/[FunctionName]/[FunctionName].tex 路径。
    """
    p = Path(src_path)
    if doc_root in p.parts and p.suffix == '.tex':
        return str(p)
    base_name = p.stem
    parts = p.parts
    idx = -1
    for name in ['src', 'include', 'doc']:
        if name in parts:
            idx = parts.index(name)
            break
    if idx != -1:
        proj_prefix = Path(*parts[:idx])
    else:
        proj_prefix = Path(".")
    return str(proj_prefix / doc_root / base_name / f"{base_name}.tex")


def fix_tex_titles_in_file(tex_path: str):
    if not os.path.exists(tex_path):
        return
    with open(tex_path, 'r', encoding='utf-8') as f:
        content = f.read()
    mapping = {
        "areaByRadius": "半径法圆面积计算",
        "areaByDiameter": "直径法圆面积计算",
        "areaByCircumference": "周长法圆面积计算",
        "areaByMonteCarlo": "蒙特卡洛法圆面积计算",
        "square": "平方运算",
    }
    modified = False
    for eng, zh in mapping.items():
        pattern_sub = f"以 {eng} 函数设计"
        target_sub = f"{eng}（{zh}）函数设计"
        if pattern_sub in content:
            content = content.replace(pattern_sub, target_sub)
            modified = True
        pattern_title = f"{eng} 函数的函数设计"
        target_title = f"{eng}（{zh}）函数设计"
        if pattern_title in content:
            content = content.replace(pattern_title, target_title)
            modified = True
        pattern_flow = f"{eng} 函数的程序流程图"
        target_flow = f"{eng} 函数程序流程图"
        if pattern_flow in content:
            content = content.replace(pattern_flow, target_flow)
            modified = True
        pattern_io = f"{eng} 函数的输入输出模块图"
        target_io = f"{eng} 函数输入输出模块图"
        if pattern_io in content:
            content = content.replace(pattern_io, target_io)
            modified = True
        pattern_arch = f"{eng} 函数的函数架构"
        target_arch = f"{eng} 函数架构图"
        if pattern_arch in content:
            content = content.replace(pattern_arch, target_arch)
            modified = True
    if modified:
        with open(tex_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"✨ 自动修复了 {tex_path} 中的标题格式与冗余词")


def clean_auxiliary_files(tex_path: str, output_dir: str):
    base_name = os.path.splitext(os.path.basename(tex_path))[0]
    for ext in ['.aux', '.log', '.toc']:
        aux_file = os.path.join(output_dir, f"{base_name}{ext}")
        if os.path.exists(aux_file):
            try:
                os.remove(aux_file)
                print(f"🧹 已清理辅助文件：{aux_file}")
            except Exception as e:
                print(f"⚠️ 清理辅助文件失败 {aux_file}: {e}")


def compile_latex(tex_path: str, output_dir: str = None):
    if output_dir is None:
        output_dir = os.path.dirname(tex_path)
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    fix_tex_titles_in_file(tex_path)
    print(f"=== 开始编译 LaTeX 文件：{tex_path} ===")
    print(f"=== 输出目录：{output_dir} ===")
    cmd1 = ['xelatex', '-interaction=nonstopmode',
            '-output-directory', output_dir, tex_path]
    result1 = subprocess.run(cmd1, capture_output=True, text=True)
    if result1.returncode != 0:
        print(f"❌ 第一次编译失败：{result1.stdout}\n{result1.stderr}")
        return None
    cmd2 = ['xelatex', '-interaction=nonstopmode',
            '-output-directory', output_dir, tex_path]
    result2 = subprocess.run(cmd2, capture_output=True, text=True)
    if result2.returncode != 0:
        print(f"❌ 第二次编译失败：{result2.stdout}\n{result2.stderr}")
        return None
    base_name = os.path.splitext(os.path.basename(tex_path))[0]
    pdf_path = os.path.join(output_dir, f"{base_name}.pdf")
    if os.path.exists(pdf_path):
        print(f"✅ 编译成功！PDF 文件：{pdf_path}")
        clean_auxiliary_files(tex_path, output_dir)
        return pdf_path
    else:
        print(f"❌ PDF 文件未生成")
        return None


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python <PromptDir>/script/compile_latex.py [tex_file.tex | src_path] [output_dir]")
        sys.exit(1)
    input_path = sys.argv[1]
    tex_file = src_to_doc_path(input_path)
    out_dir = sys.argv[2] if len(sys.argv) > 2 else None
    pdf_path = compile_latex(tex_file, out_dir)
    sys.exit(0 if pdf_path else 1)
