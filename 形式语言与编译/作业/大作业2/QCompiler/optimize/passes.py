"""QTAC 优化遍：复制传播、公共子表达式消除、死代码删除。

复制传播与 CSE 在基本块内进行；死代码删除对整个函数体进行（临时变量单赋值，
某临时全程未被读取即为死代码）。
"""


def _read_fields(ins):
    """返回该指令中作为"读操作数"的字段名。"""
    return {
        "BINOP": ["a", "b"],
        "COPY": ["a"],
        "LOAD": ["addr"],
        "STORE": ["addr", "a"],
        "IF": ["q", "a"],
        "RETURN": ["a"],
        "PAR": ["a"],
    }.get(ins.kind, [])


def _def_var(ins):
    """返回该指令定义（写入）的变量名，无则 None。"""
    if ins.kind in ("BINOP", "COPY", "LOAD", "CALL"):
        return ins.q
    return None


def copy_propagate(block):
    """复制传播：遇 x = y，把后续对 x 的引用换成 y（直到 x 被重新定义）。"""
    copy = {}
    for ins in block:
        for f in _read_fields(ins):
            v = getattr(ins, f)
            if v in copy:
                setattr(ins, f, copy[v])
        d = _def_var(ins)
        if d is not None:
            copy.pop(d, None)                                  # d 被重定义
            for k in [k for k, val in copy.items() if val == d]:
                del copy[k]                                    # 以 d 为源的复制失效
            if ins.kind == "COPY" and ins.a != d:
                copy[d] = ins.a                                # 记录新复制关系


def _invalidate(avail, var):
    """变量 var 被重定义：作废所有涉及它的可用表达式。"""
    for k in [k for k in avail if var == k[0] or var == k[2] or avail[k] == var]:
        del avail[k]


def cse(block):
    """公共子表达式消除：块内相同的算式只算一次，后续改为复用。"""
    avail = {}                          # (a, op, b) -> 持有该结果的临时
    for ins in block:
        if ins.kind == "BINOP":
            key = (ins.a, ins.op, ins.b)
            if key in avail:            # 重复算式 → 改成复制，复用之前结果
                ins.kind = "COPY"
                ins.a = avail[key]
                _invalidate(avail, ins.q)
            else:
                _invalidate(avail, ins.q)
                avail[key] = ins.q
        else:
            d = _def_var(ins)
            if d is not None:
                _invalidate(avail, d)


def dce(body):
    """死代码删除：删去结果从未被读取的纯运算/复制/取数指令（迭代至不动点）。"""
    changed = True
    while changed:
        changed = False
        used = set()
        for ins in body:
            for f in _read_fields(ins):
                used.add(getattr(ins, f))
        new = []
        for ins in body:
            d = _def_var(ins)
            if d is not None and ins.kind in ("BINOP", "COPY", "LOAD") and d not in used:
                changed = True               # 结果无人使用 → 删除
                continue
            new.append(ins)
        body[:] = new

    # 标记结果未被使用的函数调用：调用本身有副作用须保留，但结果无需保存
    used = set()
    for ins in body:
        for f in _read_fields(ins):
            used.add(getattr(ins, f))
    for ins in body:
        if ins.kind == "CALL" and ins.q not in used:
            ins.dead = True
