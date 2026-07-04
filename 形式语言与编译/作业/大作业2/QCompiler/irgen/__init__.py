"""QTAC 中间代码生成模块（步骤二里程碑2）。

对外导出：
    QTAC      —— QTAC 指令收集与文本输出
    SymbolTable / SymEntry —— 符号表
    Generator —— 遍历 QAST 生成 QTAC（语法制导）
"""

from .qtac import QTAC
from .symtab import SymbolTable, SymEntry
from .codegen import Generator

__all__ = ["QTAC", "SymbolTable", "SymEntry", "Generator"]
