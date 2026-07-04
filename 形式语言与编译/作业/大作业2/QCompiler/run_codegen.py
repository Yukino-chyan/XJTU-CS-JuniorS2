"""步骤二里程碑2 驱动：QL 源 → 词法 → 语法(QAST) → QTAC 中间代码 + 符号表。

用法：
    py run_codegen.py [输入.ql]
默认：
    输入  samples/qsort.ql
    输出  out/qsort.qtac（QTAC 三地址码）、out/symtab.txt（符号表）
"""

import os
import sys

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lexer import Lexer, LexError
from parser import read_table, Parser, ParseError
from irgen import Generator


def main():
    base = os.path.dirname(os.path.abspath(__file__))
    in_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base, "samples", "qsort.ql")
    out_qtac = os.path.join(base, "out", "qsort.qtac")
    out_sym = os.path.join(base, "out", "symtab.txt")
    table_path = os.path.join(base, "parser", "slr1_table.txt")

    with open(in_path, "r", encoding="utf-8") as f:
        source = f.read()

    try:
        tokens = Lexer(source).tokenize()
    except LexError as e:
        print(e)
        sys.exit(1)

    table = read_table(table_path)
    try:
        ast = Parser(table).parse(tokens)
    except ParseError as e:
        print(e)
        sys.exit(1)

    gen = Generator()
    qtac = gen.generate(ast)

    os.makedirs(os.path.dirname(out_qtac), exist_ok=True)
    with open(out_qtac, "w", encoding="utf-8") as f:
        f.write(qtac.text() + "\n")
    with open(out_sym, "w", encoding="utf-8") as f:
        f.write(gen.sym.dump() + "\n")

    print(qtac.text())
    print("-" * 40)
    print(f"QTAC 生成成功：共 {len(qtac.lines)} 行")
    print(f"QTAC 输出：{out_qtac}")
    print(f"符号表输出：{out_sym}")


if __name__ == "__main__":
    main()
