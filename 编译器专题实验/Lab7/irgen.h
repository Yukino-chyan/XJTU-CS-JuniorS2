#pragma once
// Lab6 中间代码生成：把 AST 翻译成四元式 (op, arg1, arg2, result)。
// Task1：四元式数据结构 + 临时变量生成 + 跳转目标(四元式序号)回填。
// 控制流采用 PPT 验收示例的“按序号跳转”形式：如 (j<, a, b, 12)、(j, , , 17)，
// 故“标号”即四元式序号，由 nextQuad() + backpatch() 管理。

#include <iostream>
#include "ast.h"

// 四元式：(op, arg1, arg2, result)
struct Quad
{
    string op;       // + - * / = ；跳转：j<(等关系) / j(无条件)
    string arg1;
    string arg2;
    string result;   // 目标变量；跳转类指令此处存目标四元式编号
};

// 中间代码生成器：维护四元式序列、临时变量计数、跳转目标回填
class IRGen
{
public:
    vector<Quad> code;          // 四元式序列；下标 i 对应编号 i+1

    string newTemp();           // 生成新临时变量：T1, T2, ...
    int    nextQuad() const;    // 下一条将生成的四元式编号（1 基）
    int    emit(const string& op, const string& a1,
                const string& a2, const string& res);   // 追加一条，返回其编号
    void   backpatch(int quadNo, int target);           // 第 quadNo 条的 result 填成目标编号

    void   gen(const ASTPtr& root);              // 语句/结构遍历入口
    string genExpr(const ASTPtr& e);             // 表达式翻译：返回 place
    int    genCondFalseJump(const ASTPtr& cond); // 发"条件为假跳转"四元式，返回编号供回填

private:
    int tempCount = 0;
};

// 便捷入口：建 IRGen → 遍历 AST → 返回四元式序列
vector<Quad> generate_ir(const ASTPtr& root);

// 输出：按 "编号. (op, arg1, arg2, result)" 逐条打印（空字段留空，对齐 PPT 示例）
void print_quads(const vector<Quad>& code, ostream& os);
