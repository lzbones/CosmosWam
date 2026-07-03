#!/usr/bin/env python3
"""
MBD FuncModule 架构合规性静态检查脚本
用法: python3 check_funcmodule_arch.py <mbd_src_dir>
"""

import sys
import os
import re
from pathlib import Path

def check_file(file_path: Path):
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    errors = []
    file_name = file_path.name
    stem = file_path.stem

    # 我们仅对头文件进行主要的类声明结构检查
    if file_path.suffix in ('.hpp', '.h'):
        # 1. 检查类继承关系：必须继承自 FuncModule<stem + "Traits">
        inherit_pattern = rf"class\s+{stem}\s*:\s*public\s+FuncModule\s*<\s*{stem}Traits\s*>"
        if not re.search(inherit_pattern, content):
            # 允许有命名空间的匹配情况
            inherit_pattern_ns = rf"class\s+{stem}\s*:\s*public\s+[a-zA-Z0-9_:]*FuncModule\s*<\s*[a-zA-Z0-9_:]*{stem}Traits\s*>"
            if not re.search(inherit_pattern_ns, content):
                errors.append(f"类 {stem} 必须继承自 FuncModule<{stem}Traits> 基类")

        # 2. 检查构造函数导入：必须包含 using FuncModule::FuncModule;
        using_pattern = r"using\s+FuncModule(?:\s*<\s*[a-zA-Z0-9_]+\s*>)?\s*::\s*FuncModule\s*;"
        if not re.search(using_pattern, content):
            # 检查是否有显式构造函数透传调用
            ctor_pattern = rf"{stem}\s*\([^)]*\)\s*:\s*FuncModule"
            if not re.search(ctor_pattern, content):
                errors.append("缺少构造函数继承声明：'using FuncModule::FuncModule;' 或显式构造函数委托")

        # 3. 检查状态、参数和子模块结构定义
        if "struct Traits" not in content and f"struct {stem}Traits" not in content:
            # 如果没有声明 Traits 结构体或继承对应 Traits，做提示
            errors.append(f"未检测到 {stem}Traits 定义")

        # 4. 检查裸指针使用（严禁使用裸指针成员）
        # 匹配任何非注释行的类型定义，包含星号 `*` 且非指针类型的特殊场景
        raw_pointer_pattern = r"(?<!//)(?<!/\*)\b[a-zA-Z0-9_]+\s*\*+\s+[a-zA-Z0-9_]+\s*;"
        if re.search(raw_pointer_pattern, content):
            errors.append("类成员中严禁定义和使用任何裸指针 (*)，应保持纯值语义状态设计")

    elif file_path.suffix in ('.cpp', '.cc'):
        # 5. 检查算法核心执行函数实现：必须有 void run(const Input&, Output&)
        # 在 .cpp 中应该实现 run 函数
        run_pattern = rf"void\s+{stem}::run\s*\(\s*const\s+Input\s*&\s*[a-zA-Z_][a-zA-Z0-9_]*\s*,\s*Output\s*&\s*[a-zA-Z_][a-zA-Z0-9_]*\s*\)"
        if not re.search(run_pattern, content):
            # 如果是扁平类成员实现，可能包含具体类型替换，如 const PIDController::Input&
            run_pattern_alt = rf"void\s+{stem}::run\s*\(\s*const\s+{stem}::Input\s*&\s*[a-zA-Z_][a-zA-Z0-9_]*\s*,\s*{stem}::Output\s*&\s*[a-zA-Z_][a-zA-Z0-9_]*\s*\)"
            if not re.search(run_pattern_alt, content):
                # 再次兜底检查声明形式
                if "void run(" not in content:
                    errors.append("必须实现核心算法接口：void run(const Input&, Output&)")

    return errors

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 check_funcmodule_arch.py <mbd_src_dir>")
        sys.exit(1)

    src_dir = Path(sys.argv[1])
    if not src_dir.exists() or not src_dir.is_dir():
        print(f"Error: Directory '{src_dir}' does not exist.")
        sys.exit(1)

    print(f"=== 开始扫描 MBD 架构合规性: {src_dir} ===")
    
    total_files = 0
    total_errors = 0
    
    # 查找所有的 .hpp 和 .cpp 文件
    for ext in ['*.hpp', '*.cpp', '*.h', '*.cc']:
        for file_path in src_dir.rglob(ext):
            total_files += 1
            errors = check_file(file_path)
            if errors:
                print(f"\n❌ 发现合规性问题：{file_path}")
                for err in errors:
                    print(f"   - {err}")
                    total_errors += 1
            else:
                print(f"✅ 合规检查通过: {file_path.name}")

    print(f"\n=== 检查完成。共扫描文件: {total_files}，发现错误数: {total_errors} ===")
    sys.exit(0 if total_errors == 0 else 1)

if __name__ == '__main__':
    main()
