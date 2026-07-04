"""QAST 抽象语法树节点（通用节点 + kind 标签）。

采用"通用节点 + kind 标签"表示素材定义的各类 QAST 结点：一个 ASTNode 类，
用 kind 标识节点种类（Block/FuncDecl/If/While/AOP/ROP/Call/ArrayAccess/…），
children 存子节点。这样既覆盖素材的节点分类，又无需为 25 种节点各写一个类，
打印与遍历都方便。

字段：
    kind   —— 节点种类
    value  —— 名字 / 字面量 / 运算符（如标识符名、整常量、'+'、'<'）
    vtype  —— 类型信息（变量/形参类型、函数返回类型；其余为空）
    children —— 子节点列表
    line/col —— 源位置（取自首个相关 token，便于后续报错）
"""


class ASTNode:
    def __init__(self, kind, value="", vtype="", children=None, line=0, col=0):
        self.kind = kind
        self.value = value
        self.vtype = vtype
        self.children = children if children is not None else []
        self.line = line
        self.col = col

    def add(self, child):
        if child is not None:
            self.children.append(child)
        return self

    def label(self):
        s = self.kind
        if self.value != "":
            s += f"({self.value})"
        if self.vtype != "":
            s += f":{self.vtype}"
        return s

    def __repr__(self):
        return self.label()


def format_tree(node, indent=0):
    """把 QAST 格式化为缩进树（每个节点一行），返回行列表。"""
    lines = ["  " * indent + node.label()]
    for c in node.children:
        lines.extend(format_tree(c, indent + 1))
    return lines


def count_nodes(node):
    return 1 + sum(count_nodes(c) for c in node.children)
