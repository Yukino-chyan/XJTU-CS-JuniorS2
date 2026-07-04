"""表驱动的 SLR(1) 移进-归约分析器，归约时建立 QAST。

驱动循环维护状态栈与属性栈：
    - 移进：压入目标状态与该终结符的叶节点；
    - 归约：弹出右部长度个符号，调用语义动作建 QAST 节点，压回，再查 GOTO；
    - 接受：属性栈顶即 QAST 根；
    - 出错：报语法错误（含行列号与"此处期望"的符号集）。

对外接口：Parser(table).parse(tokens) -> QAST 根节点。
tokens 直接取自词法器（Token.type 即文法终结符名，EOF 映射为 $）。
"""

from .semantic import make_leaf, semantic_action


class ParseError(Exception):
    def __init__(self, message, line, col):
        super().__init__(f"语法错误 (行 {line}, 列 {col}): {message}")
        self.line = line
        self.col = col


def _terminal_of(tok):
    # 词法器 token 类型名即文法终结符名；EOF 对应文法的 $
    return "$" if tok.type == "EOF" else tok.type


class Parser:
    def __init__(self, table):
        self.table = table

    def parse(self, tokens):
        action, goto, prods = self.table.action, self.table.goto, self.table.productions
        state_stack = [0]
        attr_stack = []
        ip = 0

        while True:
            s = state_stack[-1]
            tok = tokens[ip]
            a = _terminal_of(tok)
            row = action[s]

            if a not in row:
                expected = " ".join(sorted(row.keys()))
                raise ParseError(
                    f"意外的 {a} \"{tok.value}\"，此处期望 {{{expected}}}",
                    tok.line, tok.col)

            kind, val = row[a]
            if kind == "s":                         # 移进
                state_stack.append(val)
                attr_stack.append(make_leaf(a, tok.value))
                ip += 1
            elif kind == "r":                       # 归约
                prod = prods[val]
                m = len(prod.right)
                if m:
                    vals = attr_stack[-m:]
                    del attr_stack[-m:]
                    del state_stack[-m:]
                else:
                    vals = []
                attr_stack.append(semantic_action(prod, vals))
                top = state_stack[-1]
                if prod.left not in goto[top]:
                    raise ParseError(
                        f"GOTO 缺失：状态 {top} 非终结符 {prod.left}", tok.line, tok.col)
                state_stack.append(goto[top][prod.left])
            else:                                   # 接受
                return attr_stack[-1].node
