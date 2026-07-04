"""步骤1 驱动程序：读取 QL 源文件，输出 Token 序列。

用法：
    python run_lexer.py [输入.ql] [输出.txt]
默认：
    输入  samples/qsort.ql
    输出  out/tokens.txt

输出文件按指导书要求采用 (类型, 值) 格式，一行一个 Token。
终端另打印一张带行列号的对照表，便于核对与写报告。
"""

import os
import sys

# Windows 控制台默认非 UTF-8，重设 stdout 编码避免中文表头乱码（不影响输出文件）
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

# 让脚本在任意工作目录下都能 import 到 lexer 包
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lexer import Lexer, LexError


def main():
    base = os.path.dirname(os.path.abspath(__file__))
    in_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base, "samples", "qsort.ql")
    out_path = sys.argv[2] if len(sys.argv) > 2 else os.path.join(base, "out", "tokens.txt")

    with open(in_path, "r", encoding="utf-8") as f:
        source = f.read()

    try:
        tokens = Lexer(source).tokenize()
    except LexError as e:
        print(e)
        sys.exit(1)

    # 写出 (类型, 值) 格式的 token 文件
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        for tok in tokens:
            f.write(f"({tok.type}, {tok.value})\n")

    # 终端打印带行列号的对照表
    print(f"{'行:列':>8}  {'类型':<14} 值")
    print("-" * 36)
    for tok in tokens:
        pos = f"{tok.line}:{tok.col}"
        print(f"{pos:>8}  {tok.type:<14} {tok.value}")

    print("-" * 36)
    print(f"词法分析完成：共 {len(tokens)} 个 Token（含 EOF）")
    print(f"输入：{in_path}")
    print(f"输出：{out_path}")


if __name__ == "__main__":
    main()
