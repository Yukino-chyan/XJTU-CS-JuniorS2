"""ARM 后端窥孔优化：冗余跳转消除。

若一条无条件跳转 `b lX` 的目标 `lX:` 恰好紧跟其后，则该跳转多余（不跳也会落到
那里），删去即可。这能消除步骤三为简单起见在条件跳转后统一补的 `b l2`，相当于
大作业1 的 T_IF_LABEL 优化。
"""


def remove_redundant_jumps(lines):
    out = []
    removed = 0
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith("b "):                       # 无条件跳转（b.xx 不以 "b " 开头）
            target = s[2:].strip()
            nxt = lines[i + 1].strip() if i + 1 < len(lines) else ""
            if nxt == f"{target}:":                   # 目标紧跟其后 → 删除
                removed += 1
                continue
        out.append(line)
    return out, removed
