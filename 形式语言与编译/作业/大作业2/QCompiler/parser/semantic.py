"""语义动作：归约时按产生式建立对应的 QAST 节点。

每条产生式对应一个语义动作（类比 $$ = f($1,$2,…)）。这里按"左部 + 右部形态"
匹配产生式（比按编号更直观、稳健），建立 qast.ASTNode 节点。节点种类映射到素材
的 QAST 结点（AOP/ROP/AND/OR/NOT/NZ/If/While/Return/Call/ArrayAccess/…）。

属性栈元素为 SV：node 为该符号对应的 QAST 子树，lexeme 为词素原文。
"""

from .qast import ASTNode


class SV:
    """语义值（属性栈元素）：node = QAST 子树；lexeme = 词素原文。"""

    __slots__ = ("node", "lexeme")

    def __init__(self, node=None, lexeme=""):
        self.node = node
        self.lexeme = lexeme


def make_leaf(term, lexeme):
    """移进终结符时构造叶子：标识符/整常量建叶节点，其余仅保留词素。"""
    if term == "ID":
        return SV(ASTNode("ID", value=lexeme), lexeme)
    if term == "NUM":
        return SV(ASTNode("NUM", value=lexeme), lexeme)
    # 类型关键字（INT/VOID）、运算符、界符：只保留词素，不建节点
    return SV(None, lexeme)


def _splice(parent, list_node):
    """把列表节点（DeclList/StmtList/ParamList/ArgList）的子节点拼入父节点。"""
    for c in list_node.children:
        parent.children.append(c)


def semantic_action(prod, vals):
    L, R = prod.left, prod.right
    n = len(R)

    # ── 程序与块 ──
    if L == "B":                                   # B -> LBRACE Ď Š RBRACE
        node = ASTNode("Block")
        _splice(node, vals[1].node)                # 声明
        _splice(node, vals[2].node)                # 语句
        return SV(node)

    # ── 类型 ──
    if L == "T":                                   # INT / VOID
        return SV(None, vals[0].lexeme)

    # ── 声明表 / 声明 / 函数头 ──
    if L == "Ď":
        if n == 0:
            return SV(ASTNode("DeclList"))
        vals[0].node.children.append(vals[1].node)  # Ď -> Ď D
        return vals[0]

    if L == "D":
        if R[0] == "T":                            # D -> T V SEMI（变量声明）
            return SV(ASTNode("VarDecl", vtype=vals[0].lexeme, children=[vals[1].node]))
        fn = vals[0].node                          # D -> H B（函数声明）
        fn.children.append(vals[1].node)           # 追加函数体块
        return SV(fn)

    if L == "H":                                   # H -> T ID LPAREN Ǎ RPAREN
        node = ASTNode("FuncDecl", value=vals[1].lexeme, vtype=vals[0].lexeme)
        _splice(node, vals[3].node)                # 形参
        return SV(node)

    # ── 左值 / 数组访问 ──
    if L == "V":
        if n == 1:                                 # V -> ID
            return vals[0]
        if n == 3:                                 # V -> V LBRACK RBRACK（空，数组传名）
            return SV(ASTNode("ArrayAccess", value="[]", children=[vals[0].node]))
        return SV(ASTNode("ArrayAccess",           # V -> V LBRACK E RBRACK
                          children=[vals[0].node, vals[2].node]))

    # ── 形参表 / 形参 ──
    if L == "Ǎ":
        if n == 0:
            return SV(ASTNode("ParamList"))
        vals[0].node.children.append(vals[1].node)
        return vals[0]

    if L == "A":                                   # A -> T ID [LBRACK RBRACK]
        vt = vals[0].lexeme + ("[]" if n == 4 else "")
        return SV(ASTNode("Param", value=vals[1].lexeme, vtype=vt))

    # ── 语句表 / 语句 ──
    if L == "Š":
        if n == 0:
            return SV(ASTNode("StmtList"))
        vals[0].node.children.append(vals[1].node)
        return vals[0]

    if L == "S":
        if R[0] == "SEMI":                         # 空语句
            return SV(ASTNode("NOP"))
        if R[0] == "E":                            # E SEMI（表达式语句）
            return SV(ASTNode("ExprStmt", children=[vals[0].node]))
        if R[0] == "RETURN":                       # RETURN E SEMI
            return SV(ASTNode("Return", children=[vals[1].node]))
        if R[0] == "IF":
            kids = [vals[2].node, vals[4].node]
            if n == 7:                             # if-else
                kids.append(vals[6].node)
            return SV(ASTNode("If", children=kids))
        if R[0] == "WHILE":
            return SV(ASTNode("While", children=[vals[2].node, vals[4].node]))
        if R[0] == "B":                            # 块语句
            return vals[0]

    # ── 表达式（赋值/加减/乘除）──
    if L in ("E", "E1", "E2"):
        if n == 1:                                 # 直传（E->E1, E1->E2, E2->E3）
            return vals[0]
        return SV(ASTNode("AOP", value=vals[1].lexeme,
                          children=[vals[0].node, vals[2].node]))

    if L == "E3":
        if n == 1:                                 # NUM / V
            return vals[0]
        if R[0] == "LPAREN":                       # ( E )
            return vals[1]
        node = ASTNode("Call", value=vals[0].lexeme)   # ID ( Ř )
        _splice(node, vals[2].node)
        return SV(node)

    # ── 实参表 ──
    if L == "Ř":
        if n == 0:
            return SV(ASTNode("ArgList"))
        vals[0].node.children.append(vals[1].node)
        return vals[0]

    # ── 条件（|| / && / ! / 关系 / 非零）──
    if L == "C":
        if n == 1:
            return vals[0]
        return SV(ASTNode("OR", value="||", children=[vals[0].node, vals[2].node]))

    if L == "C1":
        if n == 1:
            return vals[0]
        return SV(ASTNode("AND", value="&&", children=[vals[0].node, vals[2].node]))

    if L == "C2":
        if n == 1:
            return vals[0]
        return SV(ASTNode("NOT", value="!", children=[vals[1].node]))

    if L == "C3":
        if n == 1:                                 # C3 -> E（表达式作条件，非零测试）
            return SV(ASTNode("NZ", children=[vals[0].node]))
        return SV(ASTNode("ROP", value=vals[1].lexeme,
                          children=[vals[0].node, vals[2].node]))

    # ── 增广/直传：P' -> P, P -> B ──
    if vals:
        return vals[0]
    return SV(None)
