"""ARM64 指令文本发射，把 ARM 汇编的书写格式集中于此。

指令缩进 4 空格，标号与汇编指示（.section/.global/标号）顶格。代码生成器
（codegen.py）调用这些语义化方法，不关心汇编文本细节。
"""


class ARM:
    def __init__(self):
        self.lines = []

    def _ins(self, s):
        self.lines.append("    " + s)

    # 顶格：指示与标号
    def directive(self, s):
        self.lines.append(s)

    def label(self, l):
        self.lines.append(f"{l}:")

    def raw(self, s):
        self._ins(s)

    # 数据搬运
    def mov(self, reg, imm):
        self._ins(f"mov {reg}, #{imm}")

    def adr(self, reg, sym):
        self._ins(f"adr {reg}, {sym}")

    def ldr_slot(self, reg, off):
        self._ins(f"ldr {reg}, [sp, #{off}]")

    def str_slot(self, reg, off):
        self._ins(f"str {reg}, [sp, #{off}]")

    def ldr_ind(self, reg, addr):
        self._ins(f"ldr {reg}, [{addr}]")

    def str_ind(self, reg, addr):
        self._ins(f"str {reg}, [{addr}]")

    # 运算与比较
    def aop(self, op, d, a, b):
        self._ins(f"{op} {d}, {a}, {b}")

    def cmp(self, a, b):
        self._ins(f"cmp {a}, {b}")

    # 跳转 / 调用
    def bcond(self, cc, label):
        self._ins(f"b.{cc} {label}")

    def b(self, label):
        self._ins(f"b {label}")

    def bl(self, target):
        self._ins(f"bl {target}")

    # 函数序言 / 尾声
    def prologue_push(self):
        self._ins("stp x29, x30, [sp, #-16]!")
        self._ins("mov x29, sp")

    def sub_sp(self, n):
        self._ins(f"sub sp, sp, #{n}")

    def add_sp(self, n):
        self._ins(f"add sp, sp, #{n}")

    def epilogue_pop(self):
        self._ins("ldp x29, x30, [sp], #16")

    def ret(self):
        self._ins("ret")

    def text(self):
        return "\n".join(self.lines)
