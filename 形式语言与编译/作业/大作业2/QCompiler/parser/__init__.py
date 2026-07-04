"""QL 语法分析模块（步骤二 / B 路线）。

对外导出：
    read_table —— 读取 slr1.cpp 导出的 SLR(1) 分析表
    Parser     —— 表驱动移进-归约分析器（归约时建 QAST）
    ParseError —— 语法错误异常（带行列号）
    ASTNode    —— QAST 节点（通用节点 + kind 标签）
    format_tree—— QAST 缩进树打印
"""

from .slr_table import read_table, SLRTable, Production
from .ql_parser import Parser, ParseError
from .qast import ASTNode, format_tree

__all__ = [
    "read_table", "SLRTable", "Production",
    "Parser", "ParseError", "ASTNode", "format_tree",
]
