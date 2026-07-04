"""QTAC / ARM 代码优化模块（步骤四）。

优化分两层：
  - QTAC 中端优化（与机器无关）：公共子表达式消除、复制传播、死代码删除。
    在基本块内做局部 CSE 与复制传播（安全、简单），再对整函数做死代码删除。
  - ARM 后端窥孔优化：冗余跳转消除。

对外接口：
    optimize_program(program)         —— 就地优化 QTAC（mutate program）
    program_to_text(program)          —— 结构化指令 → QTAC 文本
    remove_redundant_jumps(arm_lines) —— ARM 冗余跳转消除
"""

from .blocks import split_blocks, join_blocks
from .passes import copy_propagate, cse, dce
from .qtac_text import program_to_text, instr_to_text
from .peephole import remove_redundant_jumps

__all__ = [
    "optimize_program", "optimize_body",
    "program_to_text", "instr_to_text", "remove_redundant_jumps",
]


def optimize_body(body):
    """对一个函数体（或入口语句序列）做 QTAC 优化（就地）。"""
    blocks = split_blocks(body)
    for blk in blocks:
        # 复制传播与公共子表达式消除迭代若干轮：一轮 CSE 产生的复制经复制传播顺出
        # 后，可能暴露出新的公共子表达式（如缩放下标 i*8 复用后，arr+i*8 又成公共式），
        # 故迭代到稳定。
        for _ in range(3):
            copy_propagate(blk)
            cse(blk)
        copy_propagate(blk)
    body[:] = join_blocks(blocks)
    dce(body)                    # 整函数死代码删除（迭代）


def optimize_program(program):
    for _fb, body in program.functions:
        optimize_body(body)
    optimize_body(program.entry)
