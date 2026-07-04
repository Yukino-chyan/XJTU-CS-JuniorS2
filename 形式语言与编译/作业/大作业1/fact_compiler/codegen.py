import sys
from typing import List, Tuple, Dict, Union

# DFA 状态转移表
DELTA = {
    ("q0", "LABEL"):  "q1",
    ("q0", "q"):      "q3",
    ("q0", "GOTO"):   "q9",
    ("q0", "RETURN"): "q11",
    ("q0", "IF"):     "q13",
    ("q1", "l"):   "q2",
    ("q2", ";"):   "ACC1",
    ("q3", "="):   "q4",
    ("q4", "k"):   "q5",
    ("q4", "q"):   "q6",
    ("q5", ";"):   "ACC2",
    ("q6", ";"):   "ACC2",
    ("q6", "aop"): "q7",
    ("q7", "q"):   "q8",
    ("q7", "k"):   "q8",
    ("q8", ";"):   "ACC3",
    ("q9", "l"):   "q10",
    ("q10", ";"):  "ACC4",
    ("q11", "q"):  "q12",
    ("q11", "k"):  "q12",
    ("q12", ";"):  "ACC5",
    ("q13", "q"):     "q14",
    ("q14", "rop"):   "q15",
    ("q15", "q"):     "q16",
    ("q15", "k"):     "q16",
    ("q16", "THEN"):  "q17",
    ("q17", "l"):     "q18",
    ("q18", "ELSE"):  "q19",
    ("q19", "l"):     "q20",
    ("q20", ";"):     "q21",
    ("q21", "LABEL"): "q22",
    ("q22", "l"):     "q23",
    ("q23", ";"):     "ACC7",
}

# 附加动作表：转移时向参数缓冲区 P 写入的字段
WRITE_ACTION = {
    ("q0", "q"):     "dest",
    ("q1", "l"):     "label_name",
    ("q4", "q"):     ("src1", "q"),
    ("q4", "k"):     ("src1", "k"),
    ("q6", "aop"):   "op",
    ("q7", "q"):     ("src2", "q"),
    ("q7", "k"):     ("src2", "k"),
    ("q9", "l"):     "target",
    ("q11", "q"):    ("ret_val", "q"),
    ("q11", "k"):    ("ret_val", "k"),
    ("q13", "q"):    "lhs",
    ("q14", "rop"):  "rop",
    ("q15", "q"):    ("rhs", "q"),
    ("q15", "k"):    ("rhs", "k"),
    ("q17", "l"):    "label_true",
    ("q19", "l"):    "label_false",
    ("q22", "l"):    "label_next",
}

# 接受状态 → 模板类型
ACCEPT_LABEL = {
    "ACC1": "T_LABEL",
    "ACC2": "T_MOV",
    "ACC3": "T_AOP",
    "ACC4": "T_GOTO",
    "ACC5": "T_RETURN",
    "ACC6": "T_IF",
    "ACC7": "T_IF_LABEL",
}

# 寄存器映射表
REG_MAP = {
    "n":  "X19",
    "a":  "X20",
    "t1": "X1",
    "t2": "X2",
    "t3": "X3",
}

# 算术运算符映射
AOP_MAP = {"+": "ADD", "-": "SUB", "*": "MUL", "/": "SDIV"}

# 关系运算符映射
ROP_MAP = {"<": "B.LT", ">": "B.GT", "<=": "B.LE", ">=": "B.GE", "==": "B.EQ", "!=": "B.NE"}

# 反关系运算符映射（T_IF_LABEL 优化用）
ROP_INV_MAP = {"<": "B.GE", ">": "B.LE", "<=": "B.GT", ">=": "B.LT", "==": "B.NE", "!=": "B.EQ"}

# 词法分析：QTAC 源文本 → token 列表
def tokenize(source):
    for sym in ["<=", ">=", "==", "!="]:
        source = source.replace(sym, f" {sym} ")
    for sym in [";", "=", "<", ">", "+", "-", "*", "/"]:
        source = source.replace(sym, f" {sym} ")
    source = source.replace("<  =", "<=")
    source = source.replace(">  =", ">=")
    source = source.replace("=  =", "==")
    source = source.replace("!  =", "!=")
    return [classify(s) for s in source.split()]

# 单个字符串 → (符号类别, 字面值)
def classify(s):
    if s in ("LABEL", "IF", "THEN", "ELSE", "GOTO", "RETURN"):
        return (s, s)
    if s == "=" or s == ";":
        return (s, s)
    if s in ("+", "-", "*", "/"):
        return ("aop", s)
    if s in ("<", ">", "<=", ">=", "==", "!="):
        return ("rop", s)
    if s.startswith("l") and s[1:].isdigit():
        return ("l", s)
    if s.lstrip("-").isdigit():
        return ("k", int(s))
    return ("q", s)

# 第一遍：DFA 驱动识别，输出模板记号串
def recognize(tokens):
    output = []
    state = "q0"
    P = {}
    i = 0
    while i < len(tokens):
        sym, literal = tokens[i]
        # q21 最长匹配：非 LABEL 则接受 T_IF，当前 token 退回
        if state == "q21" and sym != "LABEL":
            output.append(("T_IF", dict(P)))
            state = "q0"
            P = {}
            continue
        key = (state, sym)
        if key not in DELTA:
            raise SyntaxError(f"非法转移: state={state}, symbol={sym}, literal={literal}")
        next_state = DELTA[key]
        if key in WRITE_ACTION:
            action = WRITE_ACTION[key]
            if isinstance(action, tuple):
                field, type_tag = action
                P[field] = literal
                P[field + "_type"] = type_tag
            else:
                P[action] = literal
        state = next_state
        i += 1
        # 到达接受状态，输出模板记号并重置
        if state in ACCEPT_LABEL:
            output.append((ACCEPT_LABEL[state], dict(P)))
            state = "q0"
            P = {}
    if state == "q21":
        output.append(("T_IF", dict(P)))
    return output

# 查寄存器映射表
def reg(name):
    return REG_MAP[name]

# 以下为各模板的 ARM 指令展开函数
def emit_label(P):
    return [f"{P['label_name']}:"]

def emit_mov(P):
    dest = reg(P['dest'])
    if P['src1_type'] == 'q':
        return [f"    MOV {dest}, {reg(P['src1'])}"]
    return [f"    MOV {dest}, #{P['src1']}"]

def emit_aop(P):
    dest, src1, arm_op = reg(P['dest']), reg(P['src1']), AOP_MAP[P['op']]
    if P['src2_type'] == 'q':
        return [f"    {arm_op} {dest}, {src1}, {reg(P['src2'])}"]
    return [f"    {arm_op} {dest}, {src1}, #{P['src2']}"]

def emit_goto(P):
    return [f"    B {P['target']}"]

def emit_return(P):
    if P['ret_val_type'] == 'q':
        return [f"    MOV X0, {reg(P['ret_val'])}", "    RET"]
    return [f"    MOV X0, #{P['ret_val']}", "    RET"]

def emit_if(P):
    lhs = reg(P['lhs'])
    rhs = reg(P['rhs']) if P['rhs_type'] == 'q' else f"#{P['rhs']}"
    return [f"    CMP {lhs}, {rhs}", f"    {ROP_MAP[P['rop']]} {P['label_true']}", f"    B {P['label_false']}"]

def emit_if_label(P):
    lhs = reg(P['lhs'])
    rhs = reg(P['rhs']) if P['rhs_type'] == 'q' else f"#{P['rhs']}"
    if P['label_next'] == P['label_false']:
        return [f"    CMP {lhs}, {rhs}", f"    {ROP_MAP[P['rop']]} {P['label_true']}", f"{P['label_false']}:"]
    return [f"    CMP {lhs}, {rhs}", f"    {ROP_INV_MAP[P['rop']]} {P['label_false']}", f"{P['label_true']}:"]

# 模板展开调度表
EMIT_DISPATCH = {
    "T_LABEL": emit_label, "T_MOV": emit_mov, "T_AOP": emit_aop,
    "T_GOTO": emit_goto, "T_RETURN": emit_return, "T_IF": emit_if, "T_IF_LABEL": emit_if_label,
}

# 第二遍：遍历模板记号串，展开为 ARM 汇编
def generate(template_tokens):
    lines = []
    for template, P in template_tokens:
        lines += EMIT_DISPATCH[template](P)
    return lines

# 输出包装：添加函数头和参数搬运
def wrap_output(body_lines):
    header = ["    .text", "    .global fact", "    .type fact, %function", "fact:"]
    prologue = ["    MOV X19, X0", "    MOV X20, X1"]  # n→X19, a→X20
    return "\n".join(header + prologue + body_lines) + "\n"

def main():
    source = sys.stdin.read()
    tokens = tokenize(source)
    templates = recognize(tokens)
    asm_lines = generate(templates)
    sys.stdout.write(wrap_output(asm_lines))

if __name__ == "__main__":
    main()