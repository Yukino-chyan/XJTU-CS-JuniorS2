#include "memory.h"
#include <cctype>

// ── 小工具 ──
static bool is_temp(const string& s)
{
    if (s.size() < 2 || s[0] != 'T') return false;
    for (size_t i = 1; i < s.size(); i++)
        if (!isdigit((unsigned char)s[i])) return false;
    return true;
}
static bool is_int_literal(const string& s)
{
    if (s.empty()) return false;
    size_t i = (s[0] == '-') ? 1 : 0;
    if (i >= s.size()) return false;
    for (; i < s.size(); i++) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}
static int round_up8(int n) { return (n + 7) / 8 * 8; }

// ── 把四元式按 (func,name,,) 切段；每段含其后续非 func 四元式的下标 ──
namespace {
struct Seg { string func; vector<int> idx; };
}
static vector<Seg> segment_quads(const vector<Quad>& q)
{
    vector<Seg> segs;
    for (int i = 0; i < (int)q.size(); i++)
    {
        if (q[i].op == "func") { Seg s; s.func = q[i].arg1; segs.push_back(s); }
        else if (!segs.empty()) segs.back().idx.push_back(i);
    }
    return segs;
}

// ── 收集一段四元式里用到的临时变量（按首次出现顺序，去重）──
static vector<string> temps_in_segment(const vector<Quad>& q, const Seg& seg)
{
    vector<string> out;
    auto consider = [&](const string& s) {
        if (!is_temp(s)) return;
        for (auto& e : out) if (e == s) return;
        out.push_back(s);
    };
    for (int i : seg.idx) { consider(q[i].arg1); consider(q[i].arg2); consider(q[i].result); }
    return out;
}

// ── 在 AST 里找名为 name 的 FUN_DECL ──
static ASTPtr find_fundecl(const ASTPtr& root, const string& name)
{
    if (!root) return nullptr;
    for (auto& c : root->children)
        if (c->node_type == AST_FUN_DECL && c->name == name) return c;
    return nullptr;
}

// ── 给一个函数（可能是合成 main）建帧 ──
static FrameInfo build_frame(const string& funcName, const ASTPtr& fundecl,
                             const vector<string>& temps)
{
    FrameInfo f;
    f.func = funcName;
    int cursor = 0;
    auto add_scalar = [&](const string& nm, const string& ty) {
        Slot s; s.name = nm; s.type = ty; s.scope = funcName;
        s.kind = STACK; s.size = 4;
        cursor += 4; s.offset = -cursor;
        f.slots.push_back(s);
    };
    auto add_array = [&](const string& nm, const string& ty, int count) {
        Slot s; s.name = nm; s.type = ty; s.scope = funcName;
        s.kind = STACK; s.array_count = count; s.size = count * 4;
        cursor += s.size; s.offset = -cursor;   // 元素0在最低地址
        f.slots.push_back(s);
    };
    auto add_ptr = [&](const string& nm, const string& ty) {   // 数组形参
        Slot s; s.name = nm; s.type = ty; s.scope = funcName;
        s.kind = STACK; s.size = 4; s.is_pointer = true;
        cursor += 4; s.offset = -cursor;
        f.slots.push_back(s);
    };

    if (fundecl)
    {
        // 形参（AST_PARAM 子节点，按序），先分配 → 拿到 -4,-8,...
        for (auto& c : fundecl->children)
            if (c->node_type == AST_PARAM)
            {
                f.nparams++;
                if (c->val_type.size() >= 2 &&
                    c->val_type.substr(c->val_type.size()-2) == "[]")
                    add_ptr(c->name, c->val_type);     // int arr[] → 指针
                else
                    add_scalar(c->name, c->val_type);
            }
        // 局部变量（AST_VAR_DECL 子节点）
        for (auto& c : fundecl->children)
            if (c->node_type == AST_VAR_DECL)
            {
                bool isArr = c->val_type.size() >= 2 &&
                             c->val_type.substr(c->val_type.size()-2) == "[]";
                if (isArr)
                {
                    int cnt = (!c->children.empty() && c->children[0]->node_type == AST_NUM)
                              ? atoi(c->children[0]->name.c_str()) : 0;
                    add_array(c->name, c->val_type, cnt);
                }
                else add_scalar(c->name, c->val_type);
            }
    }
    // 临时变量
    for (auto& t : temps) add_scalar(t, "int");

    f.frame_size = round_up8(cursor);
    return f;
}

const Slot* MemPlan::find_global(const string& name) const
{
    for (auto& g : globals) if (g.name == name) return &g;
    return nullptr;
}
const FrameInfo* MemPlan::frame(const string& func) const
{
    for (auto& f : frames) if (f.func == func) return &f;
    return nullptr;
}
const Slot* MemPlan::find(const string& func, const string& name) const
{
    if (const FrameInfo* f = frame(func))
        for (auto& s : f->slots) if (s.name == name) return &s;
    return find_global(name);
}

MemPlan plan_memory(const ASTPtr& ast_root, const vector<Quad>& quads)
{
    MemPlan plan;

    // 1) 全局变量：Program 的直接 AST_VAR_DECL 子节点
    if (ast_root)
        for (auto& c : ast_root->children)
            if (c->node_type == AST_VAR_DECL)
            {
                Slot s; s.name = c->name; s.type = c->val_type;
                s.scope = "global"; s.kind = GLOBAL; s.label = c->name;
                bool isArr = c->val_type.size() >= 2 &&
                             c->val_type.substr(c->val_type.size()-2) == "[]";
                if (isArr)
                {
                    int cnt = (!c->children.empty() && c->children[0]->node_type == AST_NUM)
                              ? atoi(c->children[0]->name.c_str()) : 0;
                    s.array_count = cnt; s.size = cnt * 4;
                }
                else
                {
                    s.size = 4;
                    if (!c->children.empty() && c->children[0]->node_type == AST_NUM
                        && is_int_literal(c->children[0]->name))
                        s.init = c->children[0]->name;     // 字面量初值
                }
                plan.globals.push_back(s);
            }

    // 2) 切段，判断有无用户 main
    vector<Seg> segs = segment_quads(quads);
    bool has_user_main = false;
    for (auto& sg : segs) if (sg.func == "main") has_user_main = true;

    // 3) 建帧
    for (auto& sg : segs)
    {
        vector<string> temps = temps_in_segment(quads, sg);
        if (sg.func == "@toplevel")
        {
            if (has_user_main) continue;                 // 丢弃顶层段
            plan.frames.push_back(build_frame("main", nullptr, temps));  // 合成 main
        }
        else
        {
            plan.frames.push_back(build_frame(sg.func, find_fundecl(ast_root, sg.func), temps));
        }
    }
    plan.entry = "main";
    return plan;
}

// ── 布局表打印（实验七验收输出）──
void print_mem_plan(const MemPlan& plan, ostream& os)
{
    os << "名字\t类型\t作用域\t存储区\t地址/偏移\t字节\n";
    for (auto& g : plan.globals)
        os << g.name << "\t" << g.type << "\tglobal\t.data\t" << g.label
           << "\t" << g.size << "\n";
    for (auto& f : plan.frames)
    {
        os << "[函数 " << f.func << "  帧大小=" << f.frame_size
           << "  形参数=" << f.nparams << "]\n";
        for (auto& s : f.slots)
            os << s.name << "\t" << s.type << "\t" << s.scope << "\tstack\tfp"
               << (s.offset < 0 ? "" : "+") << s.offset << "\t" << s.size << "\n";
    }
}
