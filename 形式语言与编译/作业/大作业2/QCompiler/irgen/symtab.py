"""符号表：作用域栈 + 条目登记，供 QTAC 生成与"符号表"产出。

每个作用域是一层字典；全局作用域含全局变量与函数，每个函数体一层（形参 + 局部
变量）。条目记录名字、种类、类型、所在作用域、数组大小。entries 为扁平登记表，
便于导出为实验要求的符号表文件。
"""


class SymEntry:
    def __init__(self, name, sort, vtype, size=0, scope="global"):
        self.name = name          # 名字
        self.sort = sort          # global / func / param / local
        self.vtype = vtype        # int / void（数组类型本身仍是元素类型）
        self.size = size          # 数组元素个数（非数组为 0）
        self.scope = scope        # 所在作用域名
        self.params = None        # 函数：形参类型列表


class SymbolTable:
    def __init__(self):
        self.scopes = [{}]            # 作用域栈，scopes[0] 为全局
        self.scope_names = ["global"]
        self.entries = []             # 扁平登记表（按声明顺序）

    def enter_scope(self, name):
        self.scopes.append({})
        self.scope_names.append(name)

    def exit_scope(self):
        self.scopes.pop()
        self.scope_names.pop()

    def declare(self, name, sort, vtype, size=0):
        entry = SymEntry(name, sort, vtype, size, self.scope_names[-1])
        self.scopes[-1][name] = entry
        self.entries.append(entry)
        return entry

    def declare_global(self, name, vtype, size=0):
        return self.declare(name, "global", vtype, size)

    def declare_func(self, name, rettype, param_types):
        entry = SymEntry(name, "func", rettype, 0, "global")
        entry.params = param_types
        self.scopes[0][name] = entry
        self.entries.append(entry)
        return entry

    def lookup(self, name):
        for scope in reversed(self.scopes):
            if name in scope:
                return scope[name]
        return None

    def dump(self):
        lines = []
        header = f"{'name':<12}{'sort':<8}{'type':<6}{'scope':<12}{'size/params'}"
        lines.append(header)
        lines.append("-" * len(header))
        for e in self.entries:
            if e.sort == "func":
                extra = "(" + ", ".join(e.params) + ")"
            else:
                extra = str(e.size) if e.size else ""
            vt = e.vtype + ("[]" if e.size and e.sort != "func" else "")
            lines.append(f"{e.name:<12}{e.sort:<8}{vt:<6}{e.scope:<12}{extra}")
        return "\n".join(lines)
