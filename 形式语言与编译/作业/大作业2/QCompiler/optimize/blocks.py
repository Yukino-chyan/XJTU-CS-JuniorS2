"""把函数体（QTAC 指令序列）划分为基本块。

基本块是单入口单出口的直线指令段。块首（leader）为：第一条指令、任何 LABEL、
以及 GOTO/IF/RETURN 之后的那条指令。局部优化（CSE、复制传播）在块内进行，
不跨块，从而无需复杂的全局数据流分析即可保证正确。
"""


def split_blocks(body):
    if not body:
        return []
    leaders = {0}
    for i, ins in enumerate(body):
        if ins.kind == "LABEL":
            leaders.add(i)
        if ins.kind in ("GOTO", "IF", "RETURN") and i + 1 < len(body):
            leaders.add(i + 1)
    starts = sorted(leaders)
    blocks = []
    for j, s in enumerate(starts):
        e = starts[j + 1] if j + 1 < len(starts) else len(body)
        blocks.append(body[s:e])
    return blocks


def join_blocks(blocks):
    return [ins for blk in blocks for ins in blk]
