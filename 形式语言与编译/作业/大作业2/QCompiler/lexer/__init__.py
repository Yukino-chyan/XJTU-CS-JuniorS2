"""QL 词法分析模块（步骤1）。

对外导出：
    Lexer    —— 词法分析器（手写 DFA 状态机）
    Token    —— Token 数据结构 (类型, 值, 行, 列)
    TokenType—— Token 类型常量集合
    LexError —— 词法错误异常（带行列号）
"""

from .tokens import Token, TokenType
from .ql_lexer import Lexer, LexError

__all__ = ["Lexer", "Token", "TokenType", "LexError"]
