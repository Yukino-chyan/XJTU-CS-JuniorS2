# Lab7 四元式 → ARMv7 汇编 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 Lab6（源码→四元式）基础上新建 Lab7 独立工程，把四元式经"内存布局（实验七）+ ARMv7 代码生成（实验八）"翻译成可经交叉编译 + QEMU 运行的 `.s` 汇编。

**Architecture:** 复用 Lab6 前端（scanner/slr1/semantic/irgen），对 irgen 与 semantic 打两处小补丁（函数边界标记 + 数组大小捕获）。新增 `memory`（AST+四元式 → MemPlan + 布局表）与 `codec`（四元式+MemPlan → ARMv7）两个模块，`lab7_main` 串起全流水线并打印每一阶段。naive 全内存 load/store，AAPCS 调用约定，只整数。

**Tech Stack:** C++17（宿主机 mingw64 g++ 编译），ARMv7 AArch32 gas 汇编；验证用 WSL 内 `arm-linux-gnueabihf-gcc -static` + `qemu-arm`。

**设计依据：** `docs/superpowers/specs/2026-05-31-lab7-arm-codegen-design.md`

> **关于提交：** 本仓库当前不是 git 仓库。若要按本计划频繁提交，先在 Task 0 执行 `git init`；否则跳过每个 `commit` 步骤即可，不影响其余步骤。

---

## 关键约定（贯穿全计划，先读）

- **四元式扩展**：irgen 在每个 `AST_FUN_DECL` 开头发 `(func, 函数名, , )`；处理 `AST_PROGRAM` 时先生成所有函数段，再发 `(func, "@toplevel", , )` 然后生成顶层语句段。于是四元式被 `func` 标记切成若干段，**顶层语句单独成一段**。
- **入口**：若用户定义了名为 `main` 的函数，它就是入口，`@toplevel` 段丢弃；否则把 `@toplevel` 段合成为 `main`。两种情况 `MemPlan` 里都有名为 `main` 的帧。
- **栈帧**：`push {fp,lr}` → `mov fp,sp` → `sub sp,sp,#FRAME`。局部/参数/临时槽在 `[fp,#负偏移]`，形参 0..3 入口从 r0..r3 spill 到前几个槽。
- **取值**：字面量用 `ldr reg,=imm`（避免立即数范围问题）；变量全局 `ldr reg,=label; ldr reg,[reg]`，栈 `ldr reg,[fp,#off]`。存值全局经 r12 中转。
- **数组**：本地/全局定长数组拥有 N*4 字节存储，元素地址 = 基址 + idx*4；数组形参 `int arr[]` 是 4 字节指针槽。
- **return**：装好 r0 后 `b .L<func>_epi`，跳到本函数唯一的 epilogue。

### 构建要点（务必遵守）

复用的前端有几个标准库文件特性：
- `slr1.cpp` 是**离线 SLR(1) 分析表生成器**（含自己的 `main`，产出 `slr1_table.txt`），**不参与流水线构建**——构建时不要包含它。
- `read_slr1_table` 实际定义在 **semantic.cpp**。
- `scanner.cpp` / `semantic.cpp` 各有一个用 `#ifndef LAB6_BUILD` 包裹的独立测试 `main`，流水线链接时必须用 **`-DLAB6_BUILD`** 排除。
- 因此流水线编译命令统一为：
  `g++ -std=c++17 -O2 -DLAB6_BUILD <模块.cpp...> <带main的主程序.cpp> -o 输出.exe`
  模块列表 = `scanner.cpp semantic.cpp irgen.cpp memory.cpp codec.cpp`（按需），主程序 = `lab7_main.cpp` 或某测试驱动。
- 先把 g++ 放上 PATH：`$env:Path = "C:\msys64\mingw64\bin;$env:Path"`。

---

## Task 0：工程骨架 + WSL 工具链

**Files:**
- Create dir: `Lab7/`（已存在，内含 PPT）
- Copy: `Lab6/{ast.h,slr1_base.h,frontend.h,scanner.cpp,slr1.cpp,semantic.cpp,irgen.h,irgen.cpp,slr1_table.txt}` → `Lab7/`
- Copy: `Lab6/代码库/` → `Lab7/代码库/`（测试用例）

- [ ] **Step 1:（可选）初始化 git**

Run（PowerShell，工作目录 `e:\2026_Junior_S2\CompilerLab`）:
```powershell
git init
"lab7`nlab7.exe`n*.o`nout.s`nprog`n_*.txt" | Out-File -Encoding utf8 .gitignore
```

- [ ] **Step 2: 复制 Lab6 源码到 Lab7**

Run（PowerShell）:
```powershell
$src = "e:\2026_Junior_S2\CompilerLab\Lab6"
$dst = "e:\2026_Junior_S2\CompilerLab\Lab7"
"ast.h","slr1_base.h","frontend.h","scanner.cpp","slr1.cpp","semantic.cpp","irgen.h","irgen.cpp","slr1_table.txt" | % {
  Copy-Item "$src\$_" "$dst\$_" -Force
}
Copy-Item "$src\代码库" "$dst\代码库" -Recurse -Force
```
Expected: `Lab7\` 下出现以上文件与 `代码库\` 目录。

- [ ] **Step 3: 冒烟编译前端（确认复用源码在 mingw64 下能编）**

Run（PowerShell，目录 `Lab7`）:
```powershell
g++ -std=c++17 -O2 -c scanner.cpp slr1.cpp semantic.cpp irgen.cpp
```
Expected: 生成 `.o` 文件，无报错（irgen.cpp 含 `#ifdef IRGEN_SELFTEST`，不带宏不会有 main 冲突）。
清理：`Remove-Item *.o`

- [ ] **Step 4: 准备 WSL ARM 工具链（验证用，一次性）**

Run:
```powershell
wsl sudo apt-get update
wsl sudo apt-get install -y gcc-arm-linux-gnueabihf qemu-user
```
验证：
```powershell
wsl arm-linux-gnueabihf-gcc --version
wsl qemu-arm --version
```
Expected: 两条都打印版本号。若 `wsl` 不可用或无网络，本任务可暂缓——后续 `.s` 生成与内存布局测试不依赖它，仅 Task 5 的 qemu 运行需要。

- [ ] **Step 5: 提交**
```bash
git add Lab7
git commit -m "chore(lab7): scaffold project by reusing Lab6 front-end"
```

---

## Task 1：前端补丁（函数边界标记 + 数组大小捕获）

**Files:**
- Modify: `Lab7/irgen.cpp`（`IRGen::gen` 的 `AST_PROGRAM` / `AST_FUN_DECL` 分支）
- Modify: `Lab7/semantic.cpp`（数组 Decl 语义动作，约 line 335）

- [ ] **Step 1: 给 irgen 加函数边界标记**

在 `Lab7/irgen.cpp` 的 `void IRGen::gen(const ASTPtr& n)` 中，把原来这段：
```cpp
        case AST_PROGRAM:
        case AST_FUN_DECL:
        case AST_COMP_STMT:
        case AST_STMT_LIST:
            for (auto& c : n->children) gen(c);   // 结构节点：递归子节点
            break;
```
替换为：
```cpp
        case AST_PROGRAM:
            // 先生成所有函数段（各自发 (func,名字,,) 标记）
            for (auto& c : n->children)
                if (c->node_type == AST_FUN_DECL) gen(c);
            // 再把顶层语句单独成段：(func,@toplevel,,) + 顶层语句
            emit("func", "@toplevel", "", "");
            for (auto& c : n->children)
                if (c->node_type != AST_FUN_DECL && c->node_type != AST_VAR_DECL)
                    gen(c);
            break;

        case AST_FUN_DECL:
            emit("func", n->name, "", "");        // 函数入口标记（= 标号名）
            for (auto& c : n->children) gen(c);   // 形参/局部声明落到 default(不发码)，语句正常生成
            break;

        case AST_COMP_STMT:
        case AST_STMT_LIST:
            for (auto& c : n->children) gen(c);
            break;
```

- [ ] **Step 2: 给 semantic 捕获数组大小**

在 `Lab7/semantic.cpp` 的 `Decl` 语义动作里，把数组分支：
```cpp
        else if (R[2] == "LBK")                         // Type ID LBK NUM RBK（数组）
            res.ast = make_node(AST_VAR_DECL, name, type + "[]");
```
替换为（把丢失的 NUM 作为子节点存进去，供 memory 读取大小）：
```cpp
        else if (R[2] == "LBK")                         // Type ID LBK NUM RBK（数组）
            res.ast = make_node(AST_VAR_DECL,
                                {make_node(AST_NUM, vals[3].lexeme)},
                                name, type + "[]");
```

- [ ] **Step 3: 编译并在一个用例上观察 func 标记**

Run（目录 `Lab7`）:
```powershell
g++ -std=c++17 -O2 -o _t.exe scanner.cpp slr1.cpp semantic.cpp irgen.cpp `
    "..\Lab6\lab6_main.cpp"
.\_t.exe 代码库\1.src
```
Expected: `=== 四元式 ===` 段里出现 `(func, main, , )` 与 `(func, @toplevel, , )` 标记行（1.src 是 `int main(){int x; x=5; return 0}; main()`）。
清理：`Remove-Item _t.exe`

> 说明：此处临时借 Lab6 的 main 只为观察四元式；Lab7 自己的 main 在 Task 4 写。

- [ ] **Step 4: 提交**
```bash
git add Lab7/irgen.cpp Lab7/semantic.cpp
git commit -m "feat(lab7): emit func-segment markers and capture array sizes in IR"
```

---

## Task 2：memory 模块（实验七：内存布局 + 布局表）

**Files:**
- Create: `Lab7/memory.h`
- Create: `Lab7/memory.cpp`

- [ ] **Step 1: 写 `Lab7/memory.h`**

```cpp
#pragma once
// Lab7 实验七：内存布局。输入 AST + 四元式，给每个全局/形参/局部/临时变量
// 分配存储位置（.data 标号 或 fp 相对偏移），产出 MemPlan + 可打印布局表。
#include "ast.h"
#include "irgen.h"
#include <ostream>

enum StoreKind { GLOBAL, STACK };

// 一个名字的存储位置
struct Slot
{
    string    name;
    string    type;            // "int" / "int[]"
    string    scope;           // 函数名 或 "global"
    StoreKind kind;
    string    label;           // GLOBAL: .data 标号（= name）
    int       offset = 0;      // STACK: fp 相对偏移（负）
    int       size   = 4;      // 字节
    bool      is_pointer = false;  // 数组形参（4 字节指针）
    int       array_count = 0;     // 拥有存储的数组元素数；标量=0
    string    init = "";       // 全局标量字面量初值（可空）
};

// 一个函数的活动记录信息
struct FrameInfo
{
    string       func;
    int          frame_size = 0;   // sub sp 的字节数（8 对齐）
    int          nparams    = 0;   // 形参个数（prologue spill）
    vector<Slot> slots;            // 形参→局部→临时（按分配顺序）
};

struct MemPlan
{
    vector<Slot>      globals;
    vector<FrameInfo> frames;      // 含合成 main
    string            entry = "main";

    const Slot*      find_global(const string& name) const;
    const Slot*      find(const string& func, const string& name) const; // 先本函数后全局
    const FrameInfo* frame(const string& func) const;
};

MemPlan plan_memory(const ASTPtr& ast_root, const vector<Quad>& quads);
void    print_mem_plan(const MemPlan& plan, ostream& os);
```

- [ ] **Step 2: 写 `Lab7/memory.cpp`**

```cpp
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
```

- [ ] **Step 3: 临时驱动测试 memory（手算对照偏移）**

新建临时文件 `Lab7/_mem_test.cpp`:
```cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include "frontend.h"
#include "irgen.h"
#include "memory.h"
using namespace std;
static string rf(const string&p){ifstream f(p,ios::binary);stringstream s;s<<f.rdbuf();return s.str();}
int main(int argc,char**argv){
    auto slr=read_slr1_table("slr1_table.txt");
    auto r=parse_and_analyze(slr,tokenize(rf(argv[1])));
    if(!r.errors.empty()){for(auto&e:r.errors)cout<<e<<"\n";return 1;}
    auto q=generate_ir(r.ast_root);
    auto plan=plan_memory(r.ast_root,q);
    print_mem_plan(plan,cout);
    return 0;
}
```
Run（目录 `Lab7`，先 `$env:Path = "C:\msys64\mingw64\bin;$env:Path"`）:
```powershell
g++ -std=c++17 -DLAB6_BUILD -o _mem.exe scanner.cpp semantic.cpp irgen.cpp memory.cpp _mem_test.cpp
.\_mem.exe 代码库\1.src
```
Expected: 打印 `[函数 main 帧大小=8 ...]`，其中局部 `x` 在 `fp-4`（1.src 只有 `int x`，可能再加 return 的临时）。手算对照：标量各 4 字节、偏移从 -4 递减、帧大小 8 对齐。
清理：`Remove-Item _mem.exe,_mem_test.cpp`

- [ ] **Step 4: 提交**
```bash
git add Lab7/memory.h Lab7/memory.cpp
git commit -m "feat(lab7): memory layout pass (globals + per-function frames + temps)"
```

---

## Task 3：codec 模块（实验八：四元式 → ARMv7）

**Files:**
- Create: `Lab7/codec.h`
- Create: `Lab7/codec.cpp`

- [ ] **Step 1: 写 `Lab7/codec.h`**

```cpp
#pragma once
// Lab7 实验八：把四元式 + MemPlan 翻译成 ARMv7 (AArch32) 汇编，写入 os。
#include "irgen.h"
#include "memory.h"
#include <ostream>

void gen_arm(const vector<Quad>& quads, const MemPlan& plan, ostream& os);
```

- [ ] **Step 2: 写 `Lab7/codec.cpp`**

```cpp
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
```

- [ ] **Step 3: 编译 codec（确保能编过）**

Run（目录 `Lab7`）:
```powershell
g++ -std=c++17 -O2 -c codec.cpp memory.cpp
```
Expected: 生成 `.o`，无报错。清理 `Remove-Item *.o`。

- [ ] **Step 4: 提交**
```bash
git add Lab7/codec.h Lab7/codec.cpp
git commit -m "feat(lab7): ARMv7 code generation from quadruples"
```

---

## Task 4：lab7_main + Makefile（全流水线串联与构建）

**Files:**
- Create: `Lab7/lab7_main.cpp`
- Create: `Lab7/Makefile`

- [ ] **Step 1: 写 `Lab7/lab7_main.cpp`**

```cpp
#include <fstream>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "frontend.h"
#include "irgen.h"
#include "memory.h"
#include "codec.h"

static string read_file(const string& path)
{
    ifstream fin(path, ios::binary);
    if (!fin.is_open()) { cerr << "无法打开源文件: " << path << endl; return ""; }
    stringstream ss; ss << fin.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    if (argc < 2)
    {
        cerr << "用法: " << argv[0]
             << " <源程序.src> [分析表=slr1_table.txt] [输出汇编=out.s]" << endl;
        return 1;
    }
    string src_path   = argv[1];
    string table_path = (argc >= 3) ? argv[2] : "slr1_table.txt";
    string asm_path   = (argc >= 4) ? argv[3] : "out.s";

    // 1) 词法
    string src = read_file(src_path);
    vector<TokenRecord> tokens = tokenize(src);
    cout << "=== Token 序列 ===" << endl;
    for (size_t i = 0; i < tokens.size(); i++)
        cout << "  " << (i + 1) << ". <" << tokens[i].type
             << ", \"" << tokens[i].lexeme << "\">" << endl;

    // 2) 语法 + 语义
    SLR1Result slr = read_slr1_table(table_path);
    SemanticResult result = parse_and_analyze(slr, tokens);
    cout << endl << "=== AST ===" << endl;        print_ast(result.ast_root, 0);
    cout << endl << "=== 符号表 ===" << endl;      print_symbol_table(result.symbols);
    cout << endl << "=== 语义检查 ===" << endl;    print_errors(result.errors);
    if (!result.errors.empty())
    {
        cout << endl << "[存在词法/语法/语义错误，停止于中间代码之前]" << endl;
        return 0;
    }

    // 3) 四元式
    cout << endl << "=== 四元式 ===" << endl;
    vector<Quad> quads = generate_ir(result.ast_root);
    print_quads(quads, cout);

    // 4) 内存布局（实验七）
    cout << endl << "=== 内存布局表 ===" << endl;
    MemPlan plan = plan_memory(result.ast_root, quads);
    print_mem_plan(plan, cout);

    // 5) ARMv7 汇编（实验八）—— 同时写文件与终端
    cout << endl << "=== ARMv7 汇编 (" << asm_path << ") ===" << endl;
    stringstream asmbuf;
    gen_arm(quads, plan, asmbuf);
    cout << asmbuf.str();
    ofstream fout(asm_path);
    if (fout.is_open()) fout << asmbuf.str();
    else cerr << "无法写汇编文件: " << asm_path << endl;

    return 0;
}
```

- [ ] **Step 2: 写 `Lab7/Makefile`**

```make
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -DLAB6_BUILD
# 注意：不含 slr1.cpp（它是离线分析表生成器，有自己的 main）；-DLAB6_BUILD 排除 scanner/semantic 的测试 main
SRC      = scanner.cpp semantic.cpp irgen.cpp memory.cpp codec.cpp lab7_main.cpp

lab7: $(SRC)
	$(CXX) $(CXXFLAGS) -o lab7 $(SRC)

clean:
	rm -f lab7 lab7.exe out.s prog *.o

.PHONY: clean
```

- [ ] **Step 3: 构建并跑一个干净用例，肉眼检查全流水**

Run（目录 `Lab7`，先 `$env:Path = "C:\msys64\mingw64\bin;$env:Path"`；msys2/WSL bash 可用 `make`，纯 PowerShell 用下面的 g++ 直编）:
```powershell
g++ -std=c++17 -O2 -DLAB6_BUILD -o lab7.exe scanner.cpp semantic.cpp irgen.cpp memory.cpp codec.cpp lab7_main.cpp
.\lab7.exe 代码库\1.src
```
Expected：依次出现 7 段：Token / AST / 符号表 / 语义检查 / 四元式 / 内存布局表 / ARMv7 汇编。汇编含 `main:` 标号、`push {fp, lr}`、`.Lmain_epi:`、结尾 `bx lr`；并生成 `Lab7\out.s`。

- [ ] **Step 4: 提交**
```bash
git add Lab7/lab7_main.cpp Lab7/Makefile
git commit -m "feat(lab7): main driver wiring full pipeline + Makefile"
```

---

## Task 5：端到端验证（WSL 交叉编译 + QEMU）与回归

**Files:**
- Create: `Lab7/tests/`（挑选的干净用例与期望，临时）

- [ ] **Step 1: 找出能通过前端的干净用例**

Run（目录 `Lab7`，PowerShell）:
```powershell
Get-ChildItem 代码库\*.src | % {
  $out = .\lab7.exe $_.FullName 2>$null | Out-String
  if ($out -match "ARMv7 汇编") { Write-Host "OK  " $_.Name }
  else { Write-Host "ERR " $_.Name }
}
```
Expected: 打印每个用例 OK/ERR。带故意错误的（如 2.src `&&`、11.src 类型错）应为 ERR（被前端 gate 拦），属正常。记下若干 OK 用例供下一步。至少应包含 1.src（int main + 赋值 + return）。

- [ ] **Step 2: 对最简用例端到端：源码 → 汇编 → 交叉编译 → qemu 运行**

先选一个能产出可见输出的干净用例。若 OK 用例里没有带 `print` 的，临时造一个 `Lab7/tests/p.src`:
```
int main() {
    int x;
    x = 7;
    print x;
    return 0
};
main()
```
Run:
```powershell
.\lab7.exe tests\p.src tests\p.s    # 生成 tests\p.s
wsl arm-linux-gnueabihf-gcc -static "tests/p.s" -o tests/p
wsl qemu-arm tests/p
```
Expected: qemu 输出 `7`（print x 走 printf "%d\n"）。
若汇编报错，按报错定位 codec（常见：标号未定义→检查跳转目标/epilogue 标号；立即数范围→已用 `ldr=` 规避）。

- [ ] **Step 3: 覆盖 if / while / 数组 / 函数调用各一个用例**

从 Step 1 的 OK 列表里各挑一个（含 `if` / `while` / 数组下标 / `d(...)` 调用），或按需在 `tests/` 造最小用例（带 `print` 以便观察）。逐个：
```powershell
.\lab7.exe tests\<用例>.src tests\<用例>.s
wsl arm-linux-gnueabihf-gcc -static "tests/<用例>.s" -o tests/<用例>
wsl qemu-arm tests/<用例>
```
Expected: 运行结果与手算预期一致。每类至少 1 个通过。

建议最小覆盖用例（放 `tests/`）：
```
// while.src：1+2+...+5 = 15
int main(){ int s; int i; s=0; i=1;
  while(i<=5){ s=s+i; i=i+1 }; print s; return 0 }; main()
```
```
// call.src：max(5,8)=8
int max(int a; int b;){ if(a<b){ return b } else { return a } };
int main(){ int r; r=max(5,8); print r; return 0 }; main()
```
```
// arr.src：a[2]=30 → print 30
int main(){ int a[3]; a[0]=10; a[1]=20; a[2]=30; print a[2]; return 0 }; main()
```

- [ ] **Step 4: 回归——前端 5 段输出与 Lab6 一致**

Run（对比同一用例在 Lab6 与 Lab7 的前 5 段；目录 `CompilerLab`）:
```powershell
.\Lab6\lab6.exe Lab6\代码库\1.src > _l6.txt
.\Lab7\lab7.exe Lab7\代码库\1.src > _l7.txt
# 比较两文件 Token..四元式 段是否一致（四元式 Lab7 多了 func 标记，属预期差异）
```
Expected: Token / AST / 符号表 / 语义检查 四段一致；四元式段 Lab7 多 `(func,…)` 标记行，符合设计。清理 `_l6.txt,_l7.txt`。

- [ ] **Step 5: 提交**
```bash
git add Lab7/tests
git commit -m "test(lab7): end-to-end ARM codegen verified via qemu (arith/if/while/array/call)"
```

---

## 完成标准（Definition of Done）

- `make`（或 g++ 直编）在宿主机产出 `lab7` 可执行。
- `lab7 某干净用例.src` 打印 7 段流水（Token→…→ARMv7 汇编），并生成 `out.s`。
- 内存布局表的偏移/字节经手算核对正确（标量 4B、数组 N*4B、8 对齐帧）。
- 至少覆盖 算术 / 赋值 / if / while / 数组 / 函数调用 / print 的用例经 WSL 交叉编译 + qemu 运行，输出与预期一致。
- 前端 5 段输出相对 Lab6 无回归（四元式多 func 标记除外）。
