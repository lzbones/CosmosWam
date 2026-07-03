#!/usr/bin/env python3
import os
import sys
import subprocess
from pathlib import Path

# 定义多个独立的平行工程，以及它们各自拆解包含的子函数/子模块列表
PROJECTS = [
    {
        "project_name": "mathLibrary", # 示例工程名（实际根据用户任务为 kalmanFilter, pidController 等）
        "functions": [
            {"name": "add", "Name": "Add"},
            {"name": "sub", "Name": "Sub"},
            {"name": "mul", "Name": "Mul"},
            {"name": "div", "Name": "Div"},
            {"name": "clamp", "Name": "Clamp"},
            {"name": "bias", "Name": "Bias"}
        ]
    }
]

ROOT = Path(__file__).parent.parent
PYTHON = str(Path("~/.ai-env/bin/python3").expanduser())

def run_cmd(args, cwd=None):
    res = subprocess.run(args, capture_output=True, text=True, cwd=cwd)
    return res.returncode, res.stdout, res.stderr

def main():
    print("=== Starting Auto Compilation and Verification Pipeline ===")
    
    summary = []
    
    for proj in PROJECTS:
        proj_name = proj["project_name"]
        print(f"\n==================================================")
        print(f"Building Project: {proj_name}")
        print(f"==================================================")
        
        module_dir = ROOT / proj_name
        if not module_dir.exists():
            print(f"❌ Project directory does not exist: {module_dir}")
            summary.append({"name": proj_name, "step": "Directory Check", "status": "FAIL", "msg": "Directory not found"})
            continue
            
        build_dir = module_dir / "build"
        build_dir.mkdir(exist_ok=True)
        
        # 1. CMake configure
        print(f"[{proj_name}] Configuring CMake...")
        code, out, err = run_cmd(["cmake", "-S", ".", "-B", "build"], cwd=module_dir)
        if code != 0:
            print(f"[{proj_name}] CMake Config Failed:\n{err}")
            summary.append({"name": proj_name, "step": "CMake Config", "status": "FAIL", "msg": err})
            continue
            
        # 2. CMake build
        print(f"[{proj_name}] Building...")
        code, out, err = run_cmd(["cmake", "--build", "build"], cwd=module_dir)
        if code != 0:
            print(f"[{proj_name}] Build Failed:\n{err}")
            summary.append({"name": proj_name, "step": "Build", "status": "FAIL", "msg": err})
            continue
            
        # 3. CTest (Runs all tests inside this project)
        print(f"[{proj_name}] Running CTest...")
        code, out, err = run_cmd(["ctest", "--output-on-failure"], cwd=build_dir)
        if code != 0:
            print(f"[{proj_name}] CTest Failed:\n{out}\n{err}")
            summary.append({"name": proj_name, "step": "CTest", "status": "FAIL", "msg": out + err})
        else:
            print(f"[{proj_name}] CTest Passed!")
            
        # 4. 遍历当前项目下的各子函数，执行绘图与设计文档编译
        for func in proj["functions"]:
            name = func["name"]
            Name = func["Name"]
            
            print(f"\n  --- Processing Function: {name} (MBD: {Name}) ---")
            
            # C++ Function Plot
            plot_func_script = module_dir / "tests" / "funcTest" / "unit" / name / "output" / f"plot_{name}.py"
            json_func_cases = module_dir / "tests" / "funcTest" / "unit" / name / f"{name}_cases.json"
            if plot_func_script.exists():
                print(f"  [{name}] Generating C++ Function Plot...")
                code, out, err = run_cmd([PYTHON, str(plot_func_script), str(json_func_cases)], cwd=module_dir)
                if code != 0:
                    print(f"  [{name}] C++ Function Plot generation failed:\n{err}")
                    summary.append({"name": f"{proj_name}:::{name}", "step": "C++ Function Plot", "status": "FAIL", "msg": err})
                else:
                    print(f"  [{name}] C++ Function Plot generated.")
            else:
                print(f"  [{name}] No C++ plot script found at: {plot_func_script.name}")
                
            # MBD Plot
            plot_mbd_script = module_dir / "tests" / "mbdTest" / "unit" / Name / "output" / f"plot_{Name}.py"
            json_mbd_cases = module_dir / "tests" / "mbdTest" / "unit" / Name / f"{Name}_cases.json"
            if plot_mbd_script.exists():
                print(f"  [{name}] Generating MBD Plot...")
                code, out, err = run_cmd([PYTHON, str(plot_mbd_script), str(json_mbd_cases)], cwd=module_dir)
                if code != 0:
                    print(f"  [{name}] MBD Plot generation failed:\n{err}")
                    summary.append({"name": f"{proj_name}:::{Name}", "step": "MBD Plot", "status": "FAIL", "msg": err})
                else:
                    print(f"  [{name}] MBD Plot generated.")
            else:
                print(f"  [{name}] No MBD plot script found at: {plot_mbd_script.name}")
            
            # Compile LaTeX to PDF
            latex_compile_script = Path(__file__).parent / "compile_latex.py"
            tex_file = module_dir / "doc" / name / f"{name}.tex"
            if tex_file.exists():
                print(f"  [{name}] Compiling LaTeX document...")
                code, out, err = run_cmd([PYTHON, str(latex_compile_script), str(tex_file)], cwd=module_dir)
                if code != 0:
                    print(f"  [{name}] LaTeX compile failed:\n{out}\n{err}")
                    summary.append({"name": f"{proj_name}:::{name}", "step": "LaTeX", "status": "FAIL", "msg": out + err})
                else:
                    print(f"  [{name}] LaTeX document compiled successfully!")
                    summary.append({"name": f"{proj_name}:::{name}", "step": "All Steps", "status": "PASS", "msg": ""})
            else:
                print(f"  [{name}] No LaTeX file found at: {tex_file.name}")
                
    print("\n==================================================")
    print("               VERIFICATION SUMMARY               ")
    print("==================================================")
    all_passed = True
    for item in summary:
        status_symbol = "✅" if item["status"] == "PASS" else "❌"
        print(f"{status_symbol} {item['name']}: {item['step']} - {item['status']}")
        if item["status"] != "PASS":
            all_passed = False
            
    print("\nFinal Result: " + ("ALL PASSED ✅" if all_passed else "SOME FAILED ❌"))

if __name__ == '__main__':
    main()
