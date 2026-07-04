"""σ-DFA 识别：把 QTAC 文本逐行识别为结构化指令并抽取操作数。

每条 QTAC 占一行，按 token 形态识别为对应指令类型（这相当于 σ-DFA 的"接受状态
→ 指令模板"）。识别结果组织为程序结构：全局声明、若干函数（定义 + 体）、入口语句。

指令类型与字段：
    GLOBAL(name, width)          FUNC_BEGIN(name, params)   FUNC_END
    LABEL(label)   GOTO(target)  IF(q, rop, a, l1, l2)
    LOAD(q, addr)  STORE(addr, a)
    BINOP(q, a, op, b)           COPY(q, a)
    PAR(a)         CALL(q, fname, k)        RETURN(a)
"""


class Instr:
    def __init__(self, kind, **kw):
        self.kind = kind
        self.__dict__.update(kw)

    def __repr__(self):
        return f"Instr({self.kind}, {self.__dict__})"


class Program:
    def __init__(self, globals_, functions, entry):
        self.globals = globals_        # [Instr(GLOBAL)]
        self.functions = functions     # [(Instr(FUNC_BEGIN), [body Instr])]
        self.entry = entry             # [body Instr]（顶层入口语句）


def parse_line(line):
    line = line.strip()
    if not line:
        return None

    # 函数框架（不以 ; 结尾）
    if line.startswith("define "):
        head = line[len("define "):]
        name = head[:head.index("(")]
        params_str = head[head.index("(") + 1: head.index(")")]
        params = [p.strip() for p in params_str.split(",") if p.strip()]
        return Instr("FUNC_BEGIN", name=name, params=params)
    if line == "}":
        return Instr("FUNC_END")

    if line.endswith(";"):
        line = line[:-1].strip()
    toks = line.split()

    if toks[0] == "LABEL":
        return Instr("LABEL", label=toks[1])
    if toks[0] == "GOTO":
        return Instr("GOTO", target=toks[1])
    if toks[0] == "IF":
        # IF q rop a THEN l1 ELSE l2
        return Instr("IF", q=toks[1], rop=toks[2], a=toks[3], l1=toks[5], l2=toks[7])
    if toks[0] == "PAR":
        return Instr("PAR", a=toks[1])
    if toks[0] == "RETURN":
        return Instr("RETURN", a=toks[1])
    # 全局声明 i<width> name
    if toks[0][0] == "i" and toks[0][1:].isdigit():
        return Instr("GLOBAL", name=toks[1], width=int(toks[0][1:]))

    # 赋值类：lhs = rhs
    if "=" in toks:
        eq = toks.index("=")
        lhs, rhs = toks[:eq], toks[eq + 1:]
        if lhs[0].startswith("M["):                 # M[q] = a
            return Instr("STORE", addr=lhs[0][2:-1], a=rhs[0])
        q = lhs[0]
        if rhs[0].startswith("M["):                 # q = M[a]
            return Instr("LOAD", q=q, addr=rhs[0][2:-1])
        if rhs[0] == "CALL":                        # q = CALL f, k
            return Instr("CALL", q=q, fname=rhs[1].rstrip(","), k=int(rhs[-1]))
        if len(rhs) == 3:                           # q = a op b
            return Instr("BINOP", q=q, a=rhs[0], op=rhs[1], b=rhs[2])
        if len(rhs) == 1:                           # q = a
            return Instr("COPY", q=q, a=rhs[0])

    raise SyntaxError(f"无法识别的 QTAC 指令：{line}")


def parse_program(text):
    globals_, functions, entry = [], [], []
    cur = None                                       # 当前函数体（None 表示在顶层）
    for raw in text.splitlines():
        ins = parse_line(raw)
        if ins is None:
            continue
        if ins.kind == "GLOBAL":
            globals_.append(ins)
        elif ins.kind == "FUNC_BEGIN":
            cur = []
            functions.append((ins, cur))
        elif ins.kind == "FUNC_END":
            cur = None
        elif cur is not None:
            cur.append(ins)
        else:
            entry.append(ins)
    return Program(globals_, functions, entry)
