"""步骤四驱动：QTAC/ARM 代码优化。

读未优化 QTAC，做中端优化（CSE/复制传播/死代码删除）得优化后 QTAC，再生成 ARM
并做窥孔优化（冗余跳转消除），输出优化后 QTAC 与 ARM。

用法：
    py run_opt.py
默认：
    输入  out/qsort.qtac
    输出  out/qsort_opt.qtac、out/qsort_opt.s
"""

import os
import sys

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from armgen import parse_program, ARMGen
from optimize import optimize_program, program_to_text, remove_redundant_jumps


def count_instrs(program):
    return sum(len(body) for _, body in program.functions) + len(program.entry)


def main():
    base = os.path.dirname(os.path.abspath(__file__))
    in_path = os.path.join(base, "out", "qsort.qtac")
    out_qtac = os.path.join(base, "out", "qsort_opt.qtac")
    out_s = os.path.join(base, "out", "qsort_opt.s")

    with open(in_path, "r", encoding="utf-8") as f:
        program = parse_program(f.read())

    # 未优化基线
    qtac_before = count_instrs(program)
    arm_before_lines = len(ARMGen(program).generate().lines)

    # 中端优化（就地）
    optimize_program(program)
    qtac_after = count_instrs(program)
    with open(out_qtac, "w", encoding="utf-8") as f:
        f.write(program_to_text(program) + "\n")

    # 重新生成 ARM + 窥孔优化
    arm = ARMGen(program).generate()
    arm_lines, removed_jumps = remove_redundant_jumps(arm.lines)
    with open(out_s, "w", encoding="utf-8") as f:
        f.write("\n".join(arm_lines) + "\n")

    print("=== 优化前后对照 ===")
    print(f"QTAC 指令数：{qtac_before} → {qtac_after}（中端优化删除 {qtac_before - qtac_after} 条）")
    print(f"ARM 行数：   {arm_before_lines} → {len(arm_lines)}"
          f"（含窥孔消除冗余跳转 {removed_jumps} 条）")
    print(f"优化后 QTAC：{out_qtac}")
    print(f"优化后 ARM： {out_s}")


if __name__ == "__main__":
    main()
