# Lab7（实验七、八合并）设计：四元式 → ARMv7 汇编

日期：2026-05-31
作者：计算机 2303 尤颢辰

## 1. 背景与目标

编译原理实验最后一次（实验七、八合并）。在 Lab6 已完成"源码 → 四元式 + 符号表"的基础上，往后端推进：

- **实验七**：根据符号表，为每个变量/数组/参数/临时变量计算**内存地址与偏移**，构建内存映像（存储布局）。
- **实验八**：根据四元式 + 实验七的内存布局，生成 **ARMv7 (32 位 AArch32) 汇编 `.s`**，可经交叉编译 + QEMU 运行。

二者是**递进关系**：实验八生成 `load/store` 与调用活动记录时，必须知道每个名字"住在内存哪里"，这正是实验七的产物。

## 2. 关键决策（设计前提）

| # | 决策 | 选择 |
|---|------|------|
| 1 | 目标指令集 | ARMv7 (32 位 AArch32)，AAPCS 调用约定 |
| 2 | 功能覆盖 | 全覆盖：算术/赋值、if/while、数组、函数调用 + 活动记录、return/print/input |
| 3 | 存储策略 | 全内存 naive load/store（变量与临时变量都有内存槽） |
| 4 | 验证方式 | arm-linux-gnueabihf-gcc 交叉编译 + qemu-arm 运行；print→printf / input→scanf（链接 libc） |
| 5 | 工程组织 | 新建 Lab7 独立工程，复用 Lab6 源码，Makefile 管理 |
| 6 | 浮点 | 只支持整数（int） |

实现思路采用**方案 A：两趟、两模块**（内存布局 / 代码生成分离），与实验七→八的递进关系一一对应。

## 3. 模块划分与数据流

### 工程目录（Lab7/，复用 Lab6 源码）

```
Lab7/
├── ast.h          ← 复用（AST/符号表/Quad 等共享结构）
├── slr1_base.h    ← 复用
├── scanner.cpp    ← 复用（词法 lexer）
├── slr1.cpp       ← 复用（SLR1 分析表读取）
├── semantic.cpp   ← 复用（语法 parser + 语义）
├── irgen.cpp/.h   ← 复用并最小扩展（见 §5：加 (func,…) 标记）
├── frontend.h     ← 复用
├── slr1_table.txt ← 复用
├── memory.h/.cpp  ← 【新增·实验七】符号表+四元式 → MemPlan + 布局表
├── codec.h/.cpp   ← 【新增·实验八】四元式+MemPlan → ARMv7 .s
├── lab7_main.cpp  ← 【新增】主程序：串起全流水线
└── Makefile       ← 【新增】make / make clean
```

### 数据流

```
源码.src
  → tokenize()            [scanner]   token 序列
  → parse_and_analyze()   [semantic]  AST + 符号表 + 语义检查
  → generate_ir()         [irgen]     vector<Quad> 四元式
  ── 以上 Lab6 已完成 ──
  → plan_memory(AST, 四元式)     [memory·实验七]  MemPlan + 内存布局表
  → gen_arm(四元式, MemPlan)     [codec·实验八]   ARMv7 .s 文本
```

### 模块职责（边界清晰，可独立测试）

- **memory**：只管"每个名字住哪儿"。输入 **AST + IR**（AST 有数组大小与清晰的按函数结构；符号表无数组大小、临时变量不在表内），输出地址映射（MemPlan）+ 人类可读布局表。不碰指令。
  - 全局：Program 的直接 `AST_VAR_DECL` 子节点。
  - 每函数：`AST_FUN_DECL` 的 `AST_PARAM`（形参，有序）+ `AST_VAR_DECL`（局部）子节点；临时变量 `Tn` 扫该函数的四元式段获得。
  - 数组大小：需给 semantic.cpp 的数组 Decl 动作打补丁，把丢失的 NUM 存进 `AST_VAR_DECL` 的子节点（见实现计划）。数组形参 `int arr[]` 视为 4 字节指针槽。
- **codec**：只管"每条四元式翻成什么指令"。查 MemPlan 取地址，不自己分配存储。
- **lab7_main**：只管把各阶段串起来、控制输出。

### 主程序输出（全流水线可见）

按顺序打印：
```
=== Token 序列 ===     ← 复用 Lab6
=== AST ===            ← 复用 Lab6
=== 符号表 ===         ← 复用 Lab6
=== 语义检查 ===       ← 复用 Lab6（有错则到此为止）
=== 四元式 ===         ← 复用 Lab6
=== 内存布局表 ===     ← 新增·实验七
=== ARMv7 汇编 (out.s) === ← 新增·实验八（同时写入 out.s）
```

## 4. 内存布局规则（实验七）

### 两个存储区

| 区 | 放什么 | 怎么分配 |
|----|--------|----------|
| `.data`（全局区） | 全局变量、全局数组 | 每名字一标号，静态地址。`int g;`→`g: .word 0`；`int g=5;`→`g: .word 5`；`int a[10];`→`a: .space 40` |
| 栈帧（活动记录） | 函数的形参、局部变量、临时变量 | `fp` 相对偏移，调用时动态分配 |

### 栈帧布局（ARMv7 AAPCS，每函数一个活动记录）

```
   高地址
   ┌────────────────┐
   │ 第5+个实参      │  [fp+8], [fp+12] …（>4 参数时，调用者压入）
   ├────────────────┤
   │ 保存的 lr       │  [fp+4]
   │ 保存的 fp       │  [fp+0]   ← fp 指向这里
   ├────────────────┤
   │ 局部/参数/临时槽 │  [fp-4], [fp-8], [fp-12] …（向下增长）
   └────────────────┘
   低地址  ← sp
```

### 偏移分配规则（每函数独立，一趟扫完它的形参+局部+临时）

- 维护 `cursor`（已用字节，从 0 起）。
- 标量：`cursor += 4`，偏移 = `-cursor`。
- 数组 `int a[N]`：元素 0 偏移 = `-(cursor + N*4)`，`a[i]` 地址 = `fp + 偏移 + 4*i`；然后 `cursor += N*4`。
- 帧大小 `FRAME` = `cursor` 向上对齐到 8 字节（AAPCS 栈 8 字节对齐）。
- 形参 0–3：入口时从 `r0-r3` 存进各自栈槽（naive，参数也落内存）。第 5 个起从 `[fp+8]` 往上读（测试用例 ≤2 参数，主用 ≤4；>4 视为已记录的扩展）。
- **分配顺序**：每函数先分配形参（保证形参拿到 fp-4, fp-8…，与 codec 的 prologue spill 顺序一致），再局部，再临时。

### MemPlan 数据结构

```cpp
enum StoreKind { GLOBAL, STACK };
struct Slot {                 // 一个名字的存储位置
    string name, type, scope;
    StoreKind kind;
    string label;             // GLOBAL: .data 标号
    int offset, size;         // STACK: fp 相对偏移(负) / 字节数
};
struct FrameInfo { string func; int frame_size; vector<Slot> slots; };
struct MemPlan {
    vector<Slot> globals;
    map<string, FrameInfo> frames;                       // 函数名 → 帧信息
    Slot* find(const string& func, const string& name);  // 查名字
};
```

### 内存布局表输出（实验七验收点）

```
名字   类型    作用域   存储区   地址/偏移   字节
a      int     max      stack    fp-4        4
b      int     max      stack    fp-8        4
T1     int     max      stack    fp-12       4
g      int     global   .data    g           4
arr    int[10] global   .data    arr         40
```

## 5. 四元式 → ARMv7 翻译规则（实验八）

### IR 最小扩展（前提）

Lab6 四元式是整程序平铺流，无函数边界标记，但栈帧按函数分。故在 `irgen` 每个 `AST_FUN_DECL` 开头多发一条 `(func, 函数名, , )`：
- memory 据此把四元式按 `func`…切段，知道哪些临时变量属于哪个帧；
- codec 据此作为函数入口（标号 + prologue）。

改 Lab7 内的 irgen 拷贝，Lab6 原件不动。

### 取/存辅助（naive 策略核心）

- `LD reg, x`：x 是字面量 → `mov reg, #x`（大数 `ldr reg, =x`）；变量/临时 → 全局 `ldr reg, =label; ldr reg, [reg]`，栈 `ldr reg, [fp, #offset]`。
- `ST reg, x`：全局 `ldr r12, =label; str reg, [r12]`，栈 `str reg, [fp, #offset]`。
- 约定：`r0/r1` 放操作数值，`r12(ip)` 做地址临时（不污染操作数）。

### 逐条规则

| 四元式 | ARMv7 |
|--------|-------|
| `(+,a,b,t)` | `LD r0,a` · `LD r1,b` · `add r0,r0,r1` · `ST r0,t` |
| `(-,a,b,t)` `(*,…)` | `sub` / `mul r0,r0,r1` |
| `(/,a,b,t)` | `LD r0,a` · `LD r1,b` · `bl __aeabi_idiv` · `ST r0,t`（借 libgcc 除法，QEMU 稳） |
| `(=,rhs,,d)` | `LD r0,rhs` · `ST r0,d` |
| `(=[],arr,idx,t)` | 取 arr 基址→r1 · `LD r0,idx` · `add r1,r1,r0,lsl #2` · `ldr r0,[r1]` · `ST r0,t` |
| `([]=,rhs,idx,arr)` | 取 arr 基址→r1 · `LD r2,idx` · `add r1,r1,r2,lsl #2` · `LD r0,rhs` · `str r0,[r1]` |
| `(j<,a,b,N)` | `LD r0,a` · `LD r1,b` · `cmp r0,r1` · `blt LN`（`<=`→ble,`>`→bgt,`>=`→bge,`==`→beq,`!=`→bne） |
| `(jz,x,,N)` | `LD r0,x` · `cmp r0,#0` · `beq LN` |
| `(j,,,N)` | `b LN` |
| `(param,x,,)` | 缓存 x |
| `(call,f,argc,t)` | 缓存的实参 `LD` 进 `r0..r{argc-1}`（地址临时用 r12）· `bl f` · t 非空则 `ST r0,t` |
| `(func,name,,)` | 函数入口：标号 + prologue |
| `(return,v,,)` | v 非空 `LD r0,v` · epilogue |
| `(print,v,,)` | `LD r1,v` · `ldr r0,=fmt_int` · `bl printf` |
| `(input,,,d)` | `ldr r0,=fmt_in` · 取 d 地址→r1 · `bl scanf` |

取 arr 基址：全局 `ldr r1, =arr`；局部 `add r1, fp, #offset`。

### 跳转标号

四元式编号全程序唯一。codec 预扫一遍收集所有跳转目标编号，在对应四元式前发 `LN:`（N=编号）。

### 函数 prologue / epilogue（活动记录）

```asm
name:                       @ (func,name,,)
    push {fp, lr}
    mov  fp, sp
    sub  sp, sp, #FRAME     @ FRAME 来自 MemPlan
    str  r0, [fp, #-4]      @ 形参 0..3 从 r0..r3 落到栈槽
    str  r1, [fp, #-8]      @ （按 argc 决定几条）
    ...
@ ── 函数体四元式 ──
    mov  sp, fp             @ epilogue（return 处 / 函数末尾兜底各发一份）
    pop  {fp, lr}
    bx   lr
```

### 程序入口（规划阶段核实后修订）

核实 `代码库/*.src` 发现真实约定是：**定义若干函数（名字任意：`test`/`func`/`acc`/`add`/`sort`，有时叫 `main`）+ 末尾一句顶层调用作为入口**（`test()` / `acc()` / `main()` …）。只有 19/33 用例叫 `main`。故入口规则修订为：

gcc+libc 链接时 libc 的 `_start` 会调用名为 `main` 的符号，所以生成代码必须产出一个 `main`：

- **若用户定义了名为 `main` 的函数** → 它就是入口（libc 调它），**顶层 StmtList 段不生成代码**（那只是冗余的 `main()` 调用）。
- **若没有用户 `main`** → 把**顶层 StmtList 段合成为 `main` 函数**（标号 `main` + prologue + 这段四元式 + epilogue），用户自定义函数照常按各自名字生成。
- 这样 33 个用例统一处理，无需重命名。

IR 切段配合：`irgen` 在每个 `AST_FUN_DECL` 开头发 `(func, 名字, , )`；`Prog -> DeclList StmtList` 中 DeclList 先 gen（各函数段），StmtList 后 gen，故**顶层语句是末尾一段没有 `func` 前缀的"尾段"**。memory/codec 据此：无用户 `main` 时把尾段当作 `main` 处理（合成帧 + 标号）；有用户 `main` 时丢弃尾段。全局 VarDecl 不发四元式（只进符号表），不影响切段。

> 负面用例说明：测试集中不少 `.src` 含**故意的词法/语法/语义错误**（如 2.src 的 `&&`、11.src 的 float 传 int 形参）。这些被前端 gate 拦截、不进入代码生成，属正常。端到端测试只选能通过前端的"干净"用例。

### `.data` 末尾固定格式串

```asm
fmt_int: .asciz "%d\n"      @ print 用
fmt_in:  .asciz "%d"        @ input 用
```

## 6. 错误处理

- 前端错误（词法/语法/语义）：沿用 Lab6 gate——`result.errors` 非空则打印错误、停在四元式之前，不进内存布局与代码生成。
- 代码生成阶段"不该发生"的情况（MemPlan 找不到名字、`func` 标记缺失、argc>4 等）：在汇编里发 `@ ERROR: ...` 注释 + stderr 打诊断，**继续生成**不崩溃。
- 假设（规划时验证）：程序含 `main`；参数 ≤4；只整数。不做数组越界等运行期检查（超范围）。

## 7. Makefile 与验证

### Makefile

```make
CXX = g++
CXXFLAGS = -std=c++17 -O2 -DLAB6_BUILD
# 不含 slr1.cpp（离线分析表生成器，自带 main）；-DLAB6_BUILD 排除 scanner/semantic 的测试 main
SRC = scanner.cpp semantic.cpp irgen.cpp memory.cpp codec.cpp lab7_main.cpp
lab7: $(SRC)
	$(CXX) $(CXXFLAGS) -o lab7 $(SRC)
clean:
	rm -f lab7 lab7.exe out.s prog *.o
```

> `lab7` 是宿主机程序（.src → .s），用普通 g++ 编。`read_slr1_table` 在 semantic.cpp；slr1.cpp 仅离线生成 `slr1_table.txt`，不进流水线构建。ARM 交叉工具链只在验证步用，不进主 Makefile，避免没装时 `make` 失败。

### 验证流程（规划阶段核实）

本机（Windows）**没有** ARM 交叉工具链与 qemu（host 的 g++ 在 msys2/mingw64），但**有 WSL**。故主程序在 Windows 用 mingw64 g++ 编译运行，ARM 汇编的验证在 **WSL** 内进行：

```bash
# 1. 主程序（Windows / mingw64 g++）：源码 → 汇编
./lab7 代码库/某用例.src > 全流水输出.txt     # 同时生成 out.s

# 2. WSL 内一次性安装工具链
wsl sudo apt-get update
wsl sudo apt-get install -y gcc-arm-linux-gnueabihf qemu-user

# 3. WSL 内交叉编译 + 运行
wsl arm-linux-gnueabihf-gcc -static out.s -o prog
wsl qemu-arm ./prog
```

`-static` 省去 qemu 动态库路径配置。若 WSL 工具链未就绪，端到端 qemu 测试跳过，但 `.s` 生成与内存布局测试仍可在 Windows 进行。

## 8. 测试计划

1. **内存布局单元测试**：挑含全局变量、局部变量、数组、多函数的用例，手算对照布局表每个偏移/地址。
2. **codec 分类测试**：按四元式类型挑最小用例（纯算术、if、while、数组读写、函数调用、print/input），逐类核对 `.s` 片段。
3. **端到端冒烟**：`代码库/` 选 3–5 代表性用例（max(a,b)/main、带 while、带数组），走完整链路交叉编译 + qemu 跑，核对输出。
4. **回归**：确认前端 5 段输出与 Lab6 一致（复用部分未改坏）。
