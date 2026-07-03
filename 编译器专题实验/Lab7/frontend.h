#pragma once
// Lab6 前端接口：把 scanner / 解析建 AST 的能力以函数形式暴露，供 lab6_main 在进程内调用。

#include "ast.h"
#include "slr1_base.h"

// 词法（scanner.cpp）：整段源码 → token 序列
vector<TokenRecord> tokenize(const string& src);

// 分析表（semantic.cpp）：读 Lab4 离线生成的 SLR(1) 分析表
SLR1Result read_slr1_table(const string& path);

// 解析 + 语义（semantic.cpp）：分析表驱动归约，建 AST + 符号表 + 类型检查
SemanticResult parse_and_analyze(const SLR1Result& slr, const vector<TokenRecord>& tokens);

// 打印辅助（semantic.cpp，验证用）
void print_ast(const ASTPtr& node, int depth);
void print_symbol_table(const vector<SymbolEntry>& symbols);
void print_errors(const vector<string>& errors);
