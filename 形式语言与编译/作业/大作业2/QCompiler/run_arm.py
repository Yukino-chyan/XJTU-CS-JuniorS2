"""步骤三驱动：QTAC → ARM64 汇编。

用法：
    py run_arm.py [输入.qtac]
默认：
    输入  out/qsort.qtac（步骤二里程碑2 产出）
    输出  out/qsort.s（ARM64 汇编，供步骤五在鲲鹏上汇编/链接/运行）
"""

import os
import sys

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from armgen import parse_program, ARMGen


def main():
    base = os.path.dirname(os.path.abspath(__file__))
    in_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base, "out", "qsort.qtac")
    out_path = os.path.join(base, "out", "qsort.s")

    with open(in_path, "r", encoding="utf-8") as f:
        text = f.read()

    program = parse_program(text)
    arm = ARMGen(program).generate()

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(arm.text() + "\n")

    print(arm.text())
    print("-" * 40)
    print(f"ARM64 汇编生成成功：共 {len(arm.lines)} 行")
    print(f"输入：{in_path}")
    print(f"输出：{out_path}")


if __name__ == "__main__":
    main()
