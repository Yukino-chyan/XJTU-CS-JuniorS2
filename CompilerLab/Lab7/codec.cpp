#include "codec.h"
#include <set>
#include <cctype>

static bool is_int_literal(const string& s)
{
    if (s.empty()) return false;
    size_t i = (s[0] == '-') ? 1 : 0;
    if (i >= s.size()) return false;
    for (; i < s.size(); i++) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

// fp 相对地址 → reg（负偏移用 sub，正用 add）
static void fp_addr(ostream& os, const string& reg, int off)
{
    if (off < 0) os << "    sub " << reg << ", fp, #" << (-off) << "\n";
    else         os << "    add " << reg << ", fp, #" << off  << "\n";
}

// 取值：x → reg
static void LD(ostream& os, const string& reg, const string& x,
               const string& func, const MemPlan& plan)
{
    if (is_int_literal(x)) { os << "    ldr " << reg << ", =" << x << "\n"; return; }
    const Slot* s = plan.find(func, x);
    if (!s) { os << "    @ ERROR: 未找到 " << x << "\n    mov " << reg << ", #0\n"; return; }
    if (s->kind == GLOBAL)
    { os << "    ldr " << reg << ", =" << s->label << "\n";
      os << "    ldr " << reg << ", [" << reg << "]\n"; }
    else
        os << "    ldr " << reg << ", [fp, #" << s->offset << "]\n";  // 标量/指针槽
}

// 存值：reg → x
static void ST(ostream& os, const string& reg, const string& x,
               const string& func, const MemPlan& plan)
{
    const Slot* s = plan.find(func, x);
    if (!s) { os << "    @ ERROR: 未找到 " << x << "\n"; return; }
    if (s->kind == GLOBAL)
    { os << "    ldr r12, =" << s->label << "\n    str " << reg << ", [r12]\n"; }
    else
        os << "    str " << reg << ", [fp, #" << s->offset << "]\n";
}

// 取地址：x 的地址 → reg（数组基址 / scanf 目标）
static void ADDR(ostream& os, const string& reg, const string& x,
                 const string& func, const MemPlan& plan)
{
    const Slot* s = plan.find(func, x);
    if (!s) { os << "    @ ERROR: 未找到 " << x << "\n    mov " << reg << ", #0\n"; return; }
    if (s->kind == GLOBAL)        os << "    ldr " << reg << ", =" << s->label << "\n";
    else if (s->is_pointer)       os << "    ldr " << reg << ", [fp, #" << s->offset << "]\n";
    else                          fp_addr(os, reg, s->offset);
}

// 收集所有跳转目标四元式编号（1 基）
static set<int> collect_targets(const vector<Quad>& q)
{
    set<int> t;
    for (auto& x : q)
        if (!x.op.empty() && x.op[0] == 'j')      // j / jz / j< / j<= ...
            t.insert(atoi(x.result.c_str()));
    return t;
}

// 关系跳转 op → 分支指令
static string branch_of(const string& op)
{
    if (op == "j<")  return "blt";
    if (op == "j<=") return "ble";
    if (op == "j>")  return "bgt";
    if (op == "j>=") return "bge";
    if (op == "j==") return "beq";
    if (op == "j!=") return "bne";
    return "b";
}

// 单条四元式 → 指令
static void emit_quad(ostream& os, const Quad& q, const string& func,
                      const MemPlan& plan, vector<string>& param_buf)
{
    const string& op = q.op;
    if (op == "+" || op == "-" || op == "*")
    {
        LD(os, "r0", q.arg1, func, plan);
        LD(os, "r1", q.arg2, func, plan);
        const char* ins = (op == "+") ? "add" : (op == "-") ? "sub" : "mul";
        os << "    " << ins << " r0, r0, r1\n";
        ST(os, "r0", q.result, func, plan);
    }
    else if (op == "/")
    {
        LD(os, "r0", q.arg1, func, plan);
        LD(os, "r1", q.arg2, func, plan);
        os << "    bl __aeabi_idiv\n";
        ST(os, "r0", q.result, func, plan);
    }
    else if (op == "=")
    {
        LD(os, "r0", q.arg1, func, plan);
        ST(os, "r0", q.result, func, plan);
    }
    else if (op == "=[]")           // t = arr[idx]
    {
        ADDR(os, "r1", q.arg1, func, plan);
        LD(os, "r0", q.arg2, func, plan);
        os << "    add r1, r1, r0, lsl #2\n    ldr r0, [r1]\n";
        ST(os, "r0", q.result, func, plan);
    }
    else if (op == "[]=")           // arr[idx] = rhs；arg1=rhs arg2=idx result=arr
    {
        ADDR(os, "r1", q.result, func, plan);
        LD(os, "r2", q.arg2, func, plan);
        os << "    add r1, r1, r2, lsl #2\n";
        LD(os, "r0", q.arg1, func, plan);
        os << "    str r0, [r1]\n";
    }
    else if (op == "jz")
    {
        LD(os, "r0", q.arg1, func, plan);
        os << "    cmp r0, #0\n    beq L" << atoi(q.result.c_str()) << "\n";
    }
    else if (op == "j")
    {
        os << "    b L" << atoi(q.result.c_str()) << "\n";
    }
    else if (!op.empty() && op[0] == 'j')          // 关系跳转
    {
        LD(os, "r0", q.arg1, func, plan);
        LD(os, "r1", q.arg2, func, plan);
        os << "    cmp r0, r1\n    " << branch_of(op) << " L"
           << atoi(q.result.c_str()) << "\n";
    }
    else if (op == "param")
    {
        param_buf.push_back(q.arg1);
    }
    else if (op == "call")
    {
        int argc = atoi(q.arg2.c_str());
        int base = (int)param_buf.size() - argc;
        if (argc > 4)
            os << "    @ ERROR: 调用 " << q.arg1
               << " 实参超过4个，本实验仅支持前4个参数经 r0-r3 传递\n";
        for (int k = 0; k < argc && k < 4; k++)
            LD(os, "r" + to_string(k), param_buf[base + k], func, plan);  // 各占一个寄存器，互不污染
        os << "    bl " << q.arg1 << "\n";
        if (!q.result.empty()) ST(os, "r0", q.result, func, plan);
        param_buf.clear();
    }
    else if (op == "return")
    {
        if (!q.arg1.empty()) LD(os, "r0", q.arg1, func, plan);
        os << "    b .L" << func << "_epi\n";
    }
    else if (op == "print")
    {
        LD(os, "r1", q.arg1, func, plan);
        os << "    ldr r0, =fmt_int\n    bl printf\n";
    }
    else if (op == "input")
    {
        os << "    ldr r0, =fmt_in\n";
        ADDR(os, "r1", q.result, func, plan);
        os << "    bl scanf\n";
    }
    else
    {
        os << "    @ ERROR: 未知 op=" << op << "\n";
    }
}

static void emit_prologue(ostream& os, const string& label,
                          const FrameInfo* f)
{
    int fs = f ? f->frame_size : 0;
    int np = f ? f->nparams : 0;
    os << label << ":\n";
    os << "    push {fp, lr}\n    mov fp, sp\n";
    if (fs > 0) os << "    sub sp, sp, #" << fs << "\n";
    for (int k = 0; k < np && k < 4; k++)         // spill 形参 r0..r3 → 槽（前 np 个）
        os << "    str r" << k << ", [fp, #" << f->slots[k].offset << "]\n";
}

static void emit_epilogue(ostream& os, const string& func)
{
    os << ".L" << func << "_epi:\n";
    os << "    mov sp, fp\n    pop {fp, lr}\n    bx lr\n";
}

void gen_arm(const vector<Quad>& quads, const MemPlan& plan, ostream& os)
{
    set<int> targets = collect_targets(quads);
    bool has_user_main = (plan.frame("main") != nullptr) &&
                         [&]{ for (auto& q : quads) if (q.op=="func" && q.arg1=="main") return true; return false; }();

    // .data
    os << "    .data\n";
    for (auto& g : plan.globals)
    {
        if (g.array_count > 0) os << g.label << ": .space " << g.size << "\n";
        else os << g.label << ": .word " << (g.init.empty() ? "0" : g.init) << "\n";
    }
    os << "fmt_int: .asciz \"%d\\n\"\n";
    os << "fmt_in:  .asciz \"%d\"\n";

    // .text
    os << "    .text\n    .global main\n";

    vector<string> param_buf;
    string cur = "";          // 当前函数（查表/标号用名）
    bool   skip = false;      // 是否在丢弃 @toplevel
    bool   synth_main = false;// 当前是合成 main（结尾补 mov r0,#0）

    for (int i = 0; i < (int)quads.size(); i++)
    {
        const Quad& q = quads[i];
        int qno = i + 1;

        if (q.op == "func")
        {
            // 关闭上一个函数
            if (!cur.empty())
            {
                if (targets.count(qno)) os << "L" << qno << ":\n"; // 落到函数末尾的跳转目标
                if (synth_main) os << "    mov r0, #0\n";
                emit_epilogue(os, cur);
            }
            cur = ""; skip = false; synth_main = false;

            string name = q.arg1;
            if (name == "@toplevel")
            {
                if (has_user_main) { skip = true; continue; }   // 丢弃
                name = "main"; synth_main = true;
            }
            cur = name;
            emit_prologue(os, name, plan.frame(name));
            continue;
        }

        if (skip) continue;
        if (targets.count(qno)) os << "L" << qno << ":\n";
        emit_quad(os, q, cur, plan, param_buf);
    }

    // 关闭最后一个函数
    if (!cur.empty())
    {
        if (targets.count((int)quads.size() + 1)) os << "L" << (quads.size()+1) << ":\n";
        if (synth_main) os << "    mov r0, #0\n";
        emit_epilogue(os, cur);
    }
}
