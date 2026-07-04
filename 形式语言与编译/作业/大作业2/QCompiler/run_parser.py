"""步骤二驱动程序：QL 源文件 → 词法分析 → SLR(1) 语法分析 → 输出 QAST 树。

用法：
    py run_parser.py [输入.ql] [输出.txt]
默认：
    输入  samples/qsort.ql
    分析表 parser/slr1_table.txt（由 slr1.cpp 导出）
    输出  out/ast.txt
"""

import os
import sys

# Windows 控制台默认非 UTF-8，重设 stdout 编码避免中文乱码
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lexer import Lexer, LexError
from parser import read_table, Parser, ParseError, format_tree
from parser.qast import count_nodes


def main():
    base = os.path.dirname(os.path.abspath(__file__))
    in_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base, "samples", "qsort.ql")
    out_path = sys.argv[2] if len(sys.argv) > 2 else os.path.join(base, "out", "ast.txt")
    table_path = os.path.join(base, "parser", "slr1_table.txt")

    with open(in_path, "r", encoding="utf-8") as f:
        source = f.read()

    # 词法分析
    try:
        tokens = Lexer(source).tokenize()
    except LexError as e:
        print(e)
        sys.exit(1)

    # 语法分析（表驱动 + 建 QAST）
    table = read_table(table_path)
    try:
        ast = Parser(table).parse(tokens)
    except ParseError as e:
        print(e)
        sys.exit(1)

    # 输出 QAST 缩进树
    tree_lines = format_tree(ast)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(tree_lines) + "\n")

    print("\n".join(tree_lines))
    print("-" * 40)
    print(f"语法分析成功：QAST 共 {count_nodes(ast)} 个节点")
    print(f"输入：{in_path}")
    print(f"输出：{out_path}")


if __name__ == "__main__":
    main()
