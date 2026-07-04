"""QTAC → ARM64 代码生成：栈槽分配 + Q2ARM 模板映射。

寄存器策略为"栈槽分配"：每个 QTAC 变量（形参/局部/临时）占一个栈槽，常量、标号、
全局数组名不占槽。每条指令把操作数加载到刷洗寄存器 x9/x10，运算后结果存回目标槽；
跨函数调用因值都在栈上而天然安全。

操作数加载规则：
    常量 k        → mov reg, #k
    全局数组名 a  → adr reg, a          （取其基址）
    变量/临时     → ldr reg, [sp, #槽]
"""

from .emitter import ARM

AOP = {"+": "add", "-": "sub", "*": "mul", "/": "sdiv"}
CC = {"<": "lt", "<=": "le", ">": "gt", ">=": "ge", "==": "eq", "!=": "ne"}


def is_const(name):
    return name.lstrip("-").isdigit()


def align16(n):
    return (n + 15) & ~15


class ARMGen:
    def __init__(self, program):
        self.p = program
        self.arm = ARM()
        self.global_names = {g.name for g in program.globals}

    # ───────────────────────── 顶层 ─────────────────────────
    def generate(self):
        # 数据段
        self.arm.directive(".section .data")
        self.arm.directive(".align 3")
        for g in self.p.globals:
            self.arm.directive(f"{g.name}: .skip {g.width}")
        # 代码段
        self.arm.directive(".section .text")
        self.arm.directive(".global _start")
        for fb, body in self.p.functions:
            self.gen_function(fb, body)
        self.gen_entry(self.p.entry)
        return self.arm

    # ─────────────────────── 栈槽分配 ───────────────────────
    def _value_operands(self, ins):
        k = ins.kind
        if k == "BINOP":  return [ins.q, ins.a, ins.b]
        if k == "COPY":   return [ins.q, ins.a]
        if k == "LOAD":   return [ins.q, ins.addr]
        if k == "STORE":  return [ins.addr, ins.a]
        if k == "IF":     return [ins.q, ins.a]
        if k == "PAR":    return [ins.a]
        if k == "CALL":   return [] if getattr(ins, "dead", False) else [ins.q]
        if k == "RETURN": return [ins.a]
        return []

    def _alloc_slots(self, params, body):
        slots = {}
        for p in params:                       # 形参优先占槽（便于序言搬运）
            slots[p] = len(slots)
        for ins in body:
            for nm in self._value_operands(ins):
                if not is_const(nm) and nm not in self.global_names and nm not in slots:
                    slots[nm] = len(slots)
        return slots

    # ─────────────────── 操作数加载 / 存储 ───────────────────
    def _load(self, name, reg, slots):
        if is_const(name):
            self.arm.mov(reg, name)
        elif name in self.global_names:
            self.arm.adr(reg, name)
        else:
            self.arm.ldr_slot(reg, slots[name] * 8)

    def _store(self, name, reg, slots):
        self.arm.str_slot(reg, slots[name] * 8)

    # ─────────────────────── 函数 ───────────────────────
    def gen_function(self, fb, body):
        name, params = fb.name, fb.params
        slots = self._alloc_slots(params, body)
        framesize = align16(len(slots) * 8)

        self.arm.label(name)
        self.arm.prologue_push()
        if framesize:
            self.arm.sub_sp(framesize)
        for i, p in enumerate(params):         # 入参 X0.. 落槽
            self.arm.str_slot(f"x{i}", slots[p] * 8)

        self._emit_body(body, slots, name, skip_label=name)

        self.arm.label(f"{name}_exit")
        if framesize:
            self.arm.add_sp(framesize)
        self.arm.epilogue_pop()
        self.arm.ret()

    def gen_entry(self, body):
        slots = self._alloc_slots([], body)
        framesize = align16(len(slots) * 8)
        self.arm.label("_start")
        if framesize:
            self.arm.sub_sp(framesize)
        self._emit_body(body, slots, None, skip_label=None)
        # exit(0) 系统调用
        self.arm.mov("x0", 0)
        self.arm.mov("x8", 93)
        self.arm.raw("svc #0")

    # ─────────────────── 指令体（含 PAR/CALL 缓冲）───────────────────
    def _emit_body(self, body, slots, fname, skip_label):
        pars = []
        for ins in body:
            k = ins.kind
            if k == "LABEL":
                if ins.label != skip_label:
                    self.arm.label(ins.label)
            elif k == "PAR":
                pars.append(ins.a)
            elif k == "CALL":
                for idx, arg in enumerate(pars):
                    self._load(arg, f"x{idx}", slots)
                self.arm.bl(ins.fname)
                if not getattr(ins, "dead", False):     # 结果未被使用则不保存
                    self._store(ins.q, "x0", slots)
                pars = []
            elif k == "GOTO":
                self.arm.b(ins.target)
            elif k == "RETURN":
                self._load(ins.a, "x0", slots)
                self.arm.b(f"{fname}_exit")
            elif k == "IF":
                self._load(ins.q, "x9", slots)
                self._load(ins.a, "x10", slots)
                self.arm.cmp("x9", "x10")
                self.arm.bcond(CC[ins.rop], ins.l1)
                self.arm.b(ins.l2)
            elif k == "BINOP":
                self._load(ins.a, "x9", slots)
                self._load(ins.b, "x10", slots)
                self.arm.aop(AOP[ins.op], "x9", "x9", "x10")
                self._store(ins.q, "x9", slots)
            elif k == "COPY":
                self._load(ins.a, "x9", slots)
                self._store(ins.q, "x9", slots)
            elif k == "LOAD":
                self._load(ins.addr, "x9", slots)
                self.arm.ldr_ind("x10", "x9")
                self._store(ins.q, "x10", slots)
            elif k == "STORE":
                self._load(ins.addr, "x9", slots)
                self._load(ins.a, "x10", slots)
                self.arm.str_ind("x10", "x9")
