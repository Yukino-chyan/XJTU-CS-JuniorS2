"""QTAC → ARM64 目标代码生成模块（步骤三）。

对外导出：
    parse_program —— σ-DFA 识别 QTAC 文本为结构化指令
    ARMGen        —— 栈槽分配 + Q2ARM 模板映射，生成 ARM64 汇编
    ARM           —— ARM 指令文本发射
"""

from .qtac_ir import parse_program, Program, Instr
from .emitter import ARM
from .codegen import ARMGen

__all__ = ["parse_program", "Program", "Instr", "ARM", "ARMGen"]
