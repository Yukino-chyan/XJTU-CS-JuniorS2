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
