"""读取 slr1.cpp 导出的 SLR(1) 分析表（parser/slr1_table.txt）。

文件格式（纯文本分节）：
    STATES <状态数>
    PRODUCTIONS <产生式数>
    <编号> <左部> <右部...>          # ε 产生式无右部
    TERMINALS <终结符... $>
    NONTERMINALS <非终结符...>
    ACTION <表项数>
    <状态> <终结符> <动作>            # sN 移进 / rN 归约 / acc 接受
    GOTO <表项数>
    <状态> <非终结符> <目标状态>
"""


class Production:
    """一条产生式：左部 + 右部符号列表（右部为空表示 ε 产生式）。"""

    def __init__(self, left, right):
        self.left = left
        self.right = right

    def __repr__(self):
        return f"{self.left} -> {' '.join(self.right) if self.right else 'ε'}"


class SLRTable:
    def __init__(self):
        self.productions = []     # 下标即产生式编号
        self.terminals = []       # 含 $
        self.nonterminals = []
        self.action = []          # action[状态][终结符] = ('s'|'r'|'acc', value)
        self.goto = []            # goto[状态][非终结符] = 目标状态


def read_table(path):
    with open(path, encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f]

    t = SLRTable()
    i = 0
    while i < len(lines):
        ln = lines[i].strip()
        i += 1
        if not ln:
            continue
        parts = ln.split()
        head = parts[0]

        if head == "STATES":
            n = int(parts[1])
            t.action = [dict() for _ in range(n)]
            t.goto = [dict() for _ in range(n)]
        elif head == "PRODUCTIONS":
            m = int(parts[1])
            for _ in range(m):
                p = lines[i].strip().split()
                i += 1
                # p[0] 是编号（与列表下标一致），p[1] 左部，p[2:] 右部
                t.productions.append(Production(p[1], p[2:]))
        elif head == "TERMINALS":
            t.terminals = parts[1:]
        elif head == "NONTERMINALS":
            t.nonterminals = parts[1:]
        elif head == "ACTION":
            c = int(parts[1])
            for _ in range(c):
                p = lines[i].strip().split()
                i += 1
                st, term, act = int(p[0]), p[1], p[2]
                if act == "acc":
                    t.action[st][term] = ("acc", 0)
                elif act[0] == "s":
                    t.action[st][term] = ("s", int(act[1:]))
                elif act[0] == "r":
                    t.action[st][term] = ("r", int(act[1:]))
        elif head == "GOTO":
            c = int(parts[1])
            for _ in range(c):
                p = lines[i].strip().split()
                i += 1
                st, nt, tgt = int(p[0]), p[1], int(p[2])
                t.goto[st][nt] = tgt

    return t
