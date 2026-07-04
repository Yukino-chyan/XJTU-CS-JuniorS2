"""遍历 QAST 生成 QTAC 三地址码（语法制导翻译）。

约定（对照素材的 qsort 参考 QTAC）：
  - 临时变量 t0,t1,… 与控制流标号 l0,l1,… 全局连续编号；
  - 表达式生成返回一个"位置"（临时变量名 / 变量名 / 常量字面量）；
  - 语句生成直接发射指令；
  - 赋值采用"目标驱动"：函数调用与数组读取的结果直接写入目标变量
    （x = CALL…；x = M[addr]），二目运算先入新临时变量再复制（t = a op b; x = t）。

对外接口：Generator().generate(ast) -> QTAC。
"""

from .qtac import QTAC
from .symtab import SymbolTable


class Generator:
    def __init__(self):
        self.q = QTAC()
        self.sym = SymbolTable()
        self.tcount = 0
        self.lcount = 0

    def new_temp(self):
        t = f"t{self.tcount}"
        self.tcount += 1
        return t

    def new_label(self):
        l = f"l{self.lcount}"
        self.lcount += 1
        return l

    # ────────────────────────── 顶层 ──────────────────────────
    def generate(self, root):
        """root 为程序根 Block：全局声明 + 函数定义 + 入口语句。"""
        globals_, funcs, entry = [], [], []
        for ch in root.children:
            if ch.kind == "VarDecl":
                globals_.append(ch)
            elif ch.kind == "FuncDecl":
                funcs.append(ch)
            else:
                entry.append(ch)

        for g in globals_:          # 全局变量声明 → i<width> name;
            self.gen_global(g)
        for f in funcs:             # 函数定义 → define …
            self.gen_function(f)
        for s in entry:             # 入口语句（如 qsort(a[],0,9,);）
            self.gen_stmt(s)
        return self.q

    def gen_global(self, vardecl):
        v = vardecl.children[0]
        if v.kind == "ArrayAccess":                  # int a[10]
            name = v.children[0].value
            size = int(v.children[1].value)
            width = size * 8                         # int 元素 8 字节（ARM64）
        else:                                        # int x
            name, size, width = v.value, 0, 8
        self.sym.declare_global(name, vardecl.vtype, size)
        self.q.glob(width, name)

    def gen_function(self, fn):
        name, rettype = fn.value, fn.vtype
        params = [c for c in fn.children if c.kind == "Param"]
        body = next(c for c in fn.children if c.kind == "Block")

        self.sym.declare_func(name, rettype, [p.vtype for p in params])
        self.sym.enter_scope(name)
        for p in params:
            self.sym.declare(p.value, "param", p.vtype)

        self.q.func_begin(name, [p.value for p in params])
        self.gen_block(body)
        if rettype == "void":               # void 函数补默认 RETURN 0
            self.q.ret("0")
        self.q.func_end()
        self.sym.exit_scope()

    # ────────────────────────── 语句 ──────────────────────────
    def gen_block(self, block):
        for ch in block.children:
            if ch.kind == "VarDecl":
                self.gen_local(ch)          # 局部声明：仅登记，不发指令
            elif ch.kind != "FuncDecl":     # QL 不允许嵌套函数
                self.gen_stmt(ch)

    def gen_local(self, vardecl):
        v = vardecl.children[0]
        if v.kind == "ArrayAccess":
            self.sym.declare(v.children[0].value, "local", vardecl.vtype,
                             int(v.children[1].value))
        else:
            self.sym.declare(v.value, "local", vardecl.vtype)

    def gen_stmt(self, s):
        k = s.kind
        if k == "NOP":
            return
        if k == "ExprStmt":
            self.gen_expr(s.children[0])
        elif k == "Return":
            self.q.ret(self.gen_expr(s.children[0]))
        elif k == "If":
            self.gen_if(s)
        elif k == "While":
            self.gen_while(s)
        elif k == "Block":
            self.gen_block(s)

    def gen_if(self, node):
        ch = node.children
        if len(ch) == 2:                    # if (C) S
            lt, lf = self.new_label(), self.new_label()
            self.gen_cond(ch[0], lt, lf)
            self.q.label(lt)
            self.gen_stmt(ch[1])
            self.q.label(lf)
        else:                               # if (C) S else S
            lt, le, lend = self.new_label(), self.new_label(), self.new_label()
            self.gen_cond(ch[0], lt, le)
            self.q.label(lt)
            self.gen_stmt(ch[1])
            self.q.goto(lend)
            self.q.label(le)
            self.gen_stmt(ch[2])
            self.q.label(lend)

    def gen_while(self, node):
        cond, body = node.children
        ltop, lbody, lexit = self.new_label(), self.new_label(), self.new_label()
        self.q.label(ltop)
        self.gen_cond(cond, lbody, lexit)   # 条件副作用在此发射
        self.q.label(lbody)
        self.gen_stmt(body)
        self.q.goto(ltop)
        self.q.label(lexit)

    # ────────────────────────── 条件 ──────────────────────────
    def gen_cond(self, c, ltrue, lfalse):
        k = c.kind
        if k == "ROP":                      # a rop b
            a = self.gen_expr(c.children[0])
            b = self.gen_expr(c.children[1])
            self.q.if_goto(a, c.value, b, ltrue, lfalse)
        elif k == "AND":                    # 短路与
            lmid = self.new_label()
            self.gen_cond(c.children[0], lmid, lfalse)
            self.q.label(lmid)
            self.gen_cond(c.children[1], ltrue, lfalse)
        elif k == "OR":                     # 短路或
            lmid = self.new_label()
            self.gen_cond(c.children[0], ltrue, lmid)
            self.q.label(lmid)
            self.gen_cond(c.children[1], ltrue, lfalse)
        elif k == "NOT":                    # 取反：交换真假出口
            self.gen_cond(c.children[0], lfalse, ltrue)
        else:                               # NZ 或表达式作条件：非零测试
            inner = c.children[0] if k == "NZ" else c
            p = self.gen_expr(inner)
            self.q.if_goto(p, "!=", "0", ltrue, lfalse)

    # ────────────────────────── 表达式 ──────────────────────────
    def gen_expr(self, e):
        k = e.kind
        if k == "NUM" or k == "ID":
            return e.value
        if k == "AOP":
            op = e.value
            if op == "=":
                return self.gen_assign(e.children[0], e.children[1])
            if op == "+=":
                a = self.gen_expr(e.children[0])
                b = self.gen_expr(e.children[1])
                t = self.new_temp()
                self.q.binop(t, a, "+", b)
                return self._store_to_target(e.children[0], t)
            a = self.gen_expr(e.children[0])         # 普通二目运算
            b = self.gen_expr(e.children[1])
            t = self.new_temp()
            self.q.binop(t, a, op, b)
            return t
        if k == "ArrayAccess":
            return self.gen_load(e, None)
        if k == "Call":
            return self.gen_call(e, None)
        raise NotImplementedError(f"gen_expr 未处理的节点：{k}")

    def gen_assign(self, target, rhs):
        """x = rhs（目标驱动）。返回所赋的值的位置（供赋值作条件用）。"""
        if target.kind == "ArrayAccess":            # arr[i] = rhs
            addr = self.gen_addr(target)
            p = self.gen_expr(rhs)
            self.q.store(addr, p)
            return p
        var = target.value                          # x = rhs（简单变量）
        if rhs.kind == "Call":
            return self.gen_call(rhs, var)          # x = CALL…
        if rhs.kind == "ArrayAccess":
            return self.gen_load(rhs, var)          # x = M[addr]
        p = self.gen_expr(rhs)                       # 二目→临时；ID/NUM→自身
        self.q.copy(var, p)                          # x = p
        return p

    def _store_to_target(self, target, place):
        if target.kind == "ArrayAccess":
            self.q.store(self.gen_addr(target), place)
        else:
            self.q.copy(target.value, place)
        return place

    def gen_addr(self, node):
        """计算数组元素地址：先把下标按元素宽度缩放，再加基址。

        元素为 8 字节的 int，故字节地址 = base + index * 8。参考 QTAC 省略了这一
        缩放（写作 base + index），在字节寻址的 ARM 上会导致元素重叠访问，故此处
        显式乘以元素宽度，生成可正确运行的地址。
        """
        base = self.gen_expr(node.children[0])
        idx = self.gen_expr(node.children[1])
        scaled = self.new_temp()
        self.q.binop(scaled, idx, "*", "8")     # 下标 × 元素宽度
        t = self.new_temp()
        self.q.binop(t, base, "+", scaled)
        return t

    def gen_load(self, node, dest):
        """数组读取：addr = base+idx; dest = M[addr]。dest 为 None 则用新临时。"""
        addr = self.gen_addr(node)
        place = dest if dest else self.new_temp()
        self.q.load(place, addr)
        return place

    def gen_call(self, node, dest):
        """函数调用：逐实参求值 → PAR → dest = CALL f, k。"""
        places = []
        for arg in node.children:
            if arg.kind == "ArrayAccess" and arg.value == "[]":
                places.append(arg.children[0].value)   # arr[] → 传基址名 arr
            else:
                places.append(self.gen_expr(arg))
        for p in places:
            self.q.par(p)
        place = dest if dest else self.new_temp()
        self.q.call(place, node.value, len(places))
        return place
