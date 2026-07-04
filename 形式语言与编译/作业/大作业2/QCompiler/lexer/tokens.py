"""QL 语言 Token 定义与词法规则表（数据驱动，便于扩展）。

本模块集中存放词法分析所需的全部"数据"，与扫描"逻辑"（ql_lexer.py）分离：
    1. TokenType        —— Token 类型常量
    2. Token            —— Token 数据结构 (类型, 值, 行, 列)
    3. KEYWORDS         —— 关键字表
    4. SINGLE_DELIMITERS—— 单字符界符表
    5. OPERATORS        —— 多字符 / 可变长运算符表

扩展点：新增或修改一个 token，只需改动这里的表，无需改动扫描器循环。
每个 Token 类型旁的注释标注了它对应的 QL 文法终结符或词素。
"""

from dataclasses import dataclass


class TokenType:
    """全部 Token 类型常量。值用字符串，便于直接写入 (类型, 值) 输出。"""

    # —— 关键字（T、S 产生式中的保留字）——
    INT = "INT"          # int
    VOID = "VOID"        # void
    IF = "IF"            # if
    ELSE = "ELSE"        # else
    WHILE = "WHILE"      # while
    RETURN = "RETURN"    # return

    # —— 标识符与常量 ——
    ID = "ID"            # 文法 d = [a-zA-Z]+
    NUM = "NUM"          # 文法 i = [0-9]+

    # —— 算术 / 赋值运算符（文法 o = {+,-,*,/,=,+=}）——
    PLUS = "PLUS"                # +
    MINUS = "MINUS"             # -
    MUL = "MUL"                 # *  （文法写作 ×，本实现以 * 为准并兼容 ×）
    DIV = "DIV"                 # /
    ASSIGN = "ASSIGN"           # =
    PLUS_ASSIGN = "PLUS_ASSIGN" # +=

    # —— 关系运算符（条件 C 中的 r）——
    LT = "LT"            # <
    LE = "LE"            # <=
    GT = "GT"            # >
    GE = "GE"            # >=
    EQ = "EQ"            # ==
    NE = "NE"            # !=

    # —— 逻辑运算符（条件 C）——
    NOT = "NOT"          # !
    AND = "AND"          # &&
    OR = "OR"            # ||

    # —— 界符 ——
    LPAREN = "LPAREN"    # (
    RPAREN = "RPAREN"    # )
    LBRACE = "LBRACE"    # {
    RBRACE = "RBRACE"    # }
    LBRACK = "LBRACK"    # [
    RBRACK = "RBRACK"    # ]
    SEMI = "SEMI"        # ;
    COMMA = "COMMA"      # ,

    # —— 输入结束 ——
    EOF = "EOF"


# 关键字表：词素 -> Token 类型。
# 扫描出一个标识符后查此表：命中即为关键字，否则按普通标识符 ID 处理。
KEYWORDS = {
    "int": TokenType.INT,
    "void": TokenType.VOID,
    "if": TokenType.IF,
    "else": TokenType.ELSE,
    "while": TokenType.WHILE,
    "return": TokenType.RETURN,
}

# 单字符界符表：字符 -> Token 类型。
SINGLE_DELIMITERS = {
    "(": TokenType.LPAREN,
    ")": TokenType.RPAREN,
    "{": TokenType.LBRACE,
    "}": TokenType.RBRACE,
    "[": TokenType.LBRACK,
    "]": TokenType.RBRACK,
    ";": TokenType.SEMI,
    ",": TokenType.COMMA,
}

# 多字符 / 可变长运算符表：首字符 -> [(期望的第二字符 或 None, 结果类型), ...]
#   - 形如 ("=", PLUS_ASSIGN) 的规则：当下一字符匹配时产出（消费两个字符）
#   - 形如 (None, PLUS) 的规则：无第二字符时的默认产出（消费一个字符）
# 扫描器按"先尝试带第二字符的规则，再退回默认规则"实现最长匹配（maximal munch）。
# '&'、'|' 没有 None 默认项，故单独出现时会触发词法错误。
# '/' 的 '//' 注释由扫描器单独识别，这里只保留单 '/' 即除号。
OPERATORS = {
    "+": [("=", TokenType.PLUS_ASSIGN), (None, TokenType.PLUS)],
    "-": [(None, TokenType.MINUS)],
    "*": [(None, TokenType.MUL)],
    "×": [(None, TokenType.MUL)],   # 兼容文法中写作 × 的乘号
    "/": [(None, TokenType.DIV)],
    "=": [("=", TokenType.EQ), (None, TokenType.ASSIGN)],
    "<": [("=", TokenType.LE), (None, TokenType.LT)],
    ">": [("=", TokenType.GE), (None, TokenType.GT)],
    "!": [("=", TokenType.NE), (None, TokenType.NOT)],
    "&": [("&", TokenType.AND)],
    "|": [("|", TokenType.OR)],
}


@dataclass
class Token:
    """一个词法单元：类型、词素值、所在行列（行列供后续阶段报错定位）。"""

    type: str
    value: str
    line: int
    col: int

    def __str__(self):
        # 指导书要求的 (类型, 值) 输出格式
        return f"({self.type}, {self.value})"
