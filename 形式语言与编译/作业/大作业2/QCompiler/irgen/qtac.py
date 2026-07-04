"""QTAC 指令的收集与文本输出。

按素材的 QTAC 文法提供各类指令的发射方法，每个方法把一条格式化好的 QTAC
文本追加到 lines。把"指令格式"集中在此处，代码生成器（codegen.py）只调用语义
方法、不关心文本细节。最终 text() 即标准 QTAC 文本（也是步骤三的输入）。

指令形式（对照素材 QTAC 定义）：
    全局:  i<width> d;
    函数:  define d(a1,…){ LABEL d; … }
    LABEL l;   GOTO l;   IF q rop a THEN l ELSE l;
    q = a;            (复制)
    q = a op b;       (二目运算 / 地址计算 q = base + idx)
    q = M[a];         (取数)      M[q] = a;   (存数)
    PAR a;   q = CALL d, k;   RETURN a;
"""


class QTAC:
    def __init__(self):
        self.lines = []

    def emit(self, line):
        self.lines.append(line)

    # ── 全局声明 / 函数框架 ──
    def glob(self, width, name):
        self.emit(f"i{width} {name};")

    def func_begin(self, name, params):
        self.emit(f"define {name}({', '.join(params)}){{")
        self.emit(f"LABEL {name};")

    def func_end(self):
        self.emit("}")

    # ── 控制流 ──
    def label(self, l):
        self.emit(f"LABEL {l};")

    def goto(self, l):
        self.emit(f"GOTO {l};")

    def if_goto(self, a, rop, b, ltrue, lfalse):
        self.emit(f"IF {a} {rop} {b} THEN {ltrue} ELSE {lfalse};")

    # ── 数据运算 / 访存 ──
    def binop(self, dst, a, op, b):
        self.emit(f"{dst} = {a} {op} {b};")

    def copy(self, dst, src):
        self.emit(f"{dst} = {src};")

    def load(self, dst, addr):
        self.emit(f"{dst} = M[{addr}];")

    def store(self, addr, src):
        self.emit(f"M[{addr}] = {src};")

    # ── 函数调用 / 返回 ──
    def par(self, a):
        self.emit(f"PAR {a};")

    def call(self, dst, fname, k):
        self.emit(f"{dst} = CALL {fname}, {k};")

    def ret(self, a):
        self.emit(f"RETURN {a};")

    def text(self):
        return "\n".join(self.lines)
