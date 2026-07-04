"""把优化后的结构化 QTAC 指令序列化回 QTAC 文本（输出优化后 QTAC、供报告对照）。"""


def instr_to_text(ins):
    k = ins.kind
    if k == "GLOBAL":     return f"i{ins.width} {ins.name};"
    if k == "FUNC_BEGIN": return f"define {ins.name}({', '.join(ins.params)}){{"
    if k == "LABEL":      return f"LABEL {ins.label};"
    if k == "GOTO":       return f"GOTO {ins.target};"
    if k == "IF":         return f"IF {ins.q} {ins.rop} {ins.a} THEN {ins.l1} ELSE {ins.l2};"
    if k == "LOAD":       return f"{ins.q} = M[{ins.addr}];"
    if k == "STORE":      return f"M[{ins.addr}] = {ins.a};"
    if k == "BINOP":      return f"{ins.q} = {ins.a} {ins.op} {ins.b};"
    if k == "COPY":       return f"{ins.q} = {ins.a};"
    if k == "PAR":        return f"PAR {ins.a};"
    if k == "CALL":       return f"{ins.q} = CALL {ins.fname}, {ins.k};"
    if k == "RETURN":     return f"RETURN {ins.a};"
    return ""


def program_to_text(program):
    lines = []
    for g in program.globals:
        lines.append(instr_to_text(g))
    for fb, body in program.functions:
        lines.append(instr_to_text(fb))
        for ins in body:
            lines.append(instr_to_text(ins))
        lines.append("}")
    for ins in program.entry:
        lines.append(instr_to_text(ins))
    return "\n".join(lines)
