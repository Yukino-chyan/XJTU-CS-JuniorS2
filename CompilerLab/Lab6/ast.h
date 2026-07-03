#pragma once
// Lab6 共享前端数据结构：AST 节点、符号表、token/语义结果。
// 从 Lab5 semantic.cpp 抽出，供 scanner(tokenize)、semantic(解析建 AST)、irgen(中间代码) 共用。

#include <string>
#include <vector>
#include <memory>
#include <map>
using namespace std;

// AST 节点种类
enum ASTNodeType
{
    AST_PROGRAM, AST_DECL_LIST,
    AST_VAR_DECL, AST_FUN_DECL, AST_PARAM, AST_PARAM_LIST,
    AST_COMP_STMT, AST_STMT_LIST,
    AST_IF_STMT, AST_WHILE_STMT, AST_RETURN_STMT, AST_PRINT_STMT, AST_INPUT_STMT,
    AST_EXPR_STMT,
    AST_ASSIGN, AST_BINOP, AST_CALL, AST_ARRAY_ACCESS,
    AST_ID, AST_NUM, AST_FLO, AST_TYPE,
    AST_ARG_LIST, AST_RELOP, AST_EMPTY
};

// 节点种类 → 可读名称（打印用）
inline string node_type_name(ASTNodeType t)
{
    switch (t)
    {
        case AST_PROGRAM:      return "Program";
        case AST_DECL_LIST:    return "DeclList";
        case AST_VAR_DECL:     return "VarDecl";
        case AST_FUN_DECL:     return "FunDecl";
        case AST_PARAM:        return "Param";
        case AST_PARAM_LIST:   return "ParamList";
        case AST_COMP_STMT:    return "CompStmt";
        case AST_STMT_LIST:    return "StmtList";
        case AST_IF_STMT:      return "IfStmt";
        case AST_WHILE_STMT:   return "WhileStmt";
        case AST_RETURN_STMT:  return "ReturnStmt";
        case AST_PRINT_STMT:   return "PrintStmt";
        case AST_INPUT_STMT:   return "InputStmt";
        case AST_EXPR_STMT:    return "ExprStmt";
        case AST_ASSIGN:       return "Assign";
        case AST_BINOP:        return "BinOp";
        case AST_CALL:         return "Call";
        case AST_ARRAY_ACCESS: return "ArrayAccess";
        case AST_ID:           return "Id";
        case AST_NUM:          return "Num";
        case AST_FLO:          return "Flo";
        case AST_TYPE:         return "Type";
        case AST_ARG_LIST:     return "ArgList";
        case AST_RELOP:        return "RelOp";
        case AST_EMPTY:        return "Empty";
    }
    return "?";
}

// AST 节点
struct ASTNode
{
    ASTNodeType node_type;
    string name;        // 标识符名 / 运算符 / 类型 / 字面量
    string val_type;    // 语义类型："int"/"float"/"void"/""
    vector<shared_ptr<ASTNode>> children;
};
using ASTPtr = shared_ptr<ASTNode>;

// 构造叶节点 / 无子节点的内部节点
inline ASTPtr make_node(ASTNodeType t, const string& name = "", const string& vt = "")
{
    auto n = make_shared<ASTNode>();
    n->node_type = t; n->name = name; n->val_type = vt;
    return n;
}

// 构造带子节点的内部节点（跳过空子节点）
inline ASTPtr make_node(ASTNodeType t, vector<ASTPtr> children,
                        const string& name = "", const string& vt = "")
{
    auto n = make_node(t, name, vt);
    for (auto& c : children) if (c) n->children.push_back(c);
    return n;
}

// 语义值：属性栈元素
struct SemanticValue
{
    ASTPtr ast;      // 该符号对应的 AST 子树
    string type;     // 表达式类型
    string lexeme;   // 词素原文
};

// 符号表条目
struct SymbolEntry
{
    string name;
    string type;          // "int"/"float"/"void"，数组加 "[]"
    int    scope_level;   // 0=全局，1=函数体…
    string scope;         // 作用域标签（打印用）
};

// 符号表：作用域栈
struct SymbolTable
{
    vector<map<string, SymbolEntry>> scopes;
    void enter_scope();
    void exit_scope();
    bool declare(const string& name, const string& type);
    SymbolEntry* lookup(const string& name);
    int current_level() const { return (int)scopes.size() - 1; }
};

struct TokenRecord { string type, lexeme; };
struct SemanticResult { ASTPtr ast_root; vector<SymbolEntry> symbols; vector<string> errors; };
