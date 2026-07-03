#pragma once
// AST + 符号表 + 语义分析（裸指针版，供 flex/bison 选做复用，逻辑同 Lab5）。

#include <string>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

// AST 节点种类
enum ASTNodeType
{
    AST_PROGRAM, AST_DECL_LIST,
    AST_VAR_DECL, AST_FUN_DECL, AST_PARAM, AST_PARAM_LIST,
    AST_COMP_STMT, AST_STMT_LIST,
    AST_IF_STMT, AST_WHILE_STMT, AST_RETURN_STMT, AST_PRINT_STMT,
    AST_EXPR_STMT,
    AST_ASSIGN, AST_BINOP, AST_CALL, AST_ARRAY_ACCESS,
    AST_ID, AST_NUM, AST_FLO, AST_TYPE,
    AST_ARG_LIST, AST_RELOP, AST_EMPTY
};

// 节点种类 → 名称（打印用）
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
    ASTNodeType      node_type;
    string           name;
    string           val_type;
    vector<ASTNode*> children;
};

// 构造节点 / 挂子节点（供 lab5.y 动作调用）
inline ASTNode* make_node(ASTNodeType t, const char* name = "", const char* vt = "")
{
    ASTNode* n = new ASTNode();
    n->node_type = t;
    n->name      = name ? name : "";
    n->val_type  = vt   ? vt   : "";
    return n;
}

inline void add_child(ASTNode* parent, ASTNode* child)
{
    if (child) parent->children.push_back(child);
}

inline void splice(ASTNode* parent, ASTNode* list)   // 摊平 list 容器的子节点
{
    if (list) for (ASTNode* c : list->children) parent->children.push_back(c);
}

inline ASTNode* make_binop(const char* op, ASTNode* l, ASTNode* r)
{
    ASTNode* n = make_node(AST_BINOP, op);           // 类型由 analyze 计算
    add_child(n, l); add_child(n, r);
    return n;
}

// 符号表条目
struct SymbolEntry { string name, type; int scope_level; string scope; };

// 符号表：作用域栈
struct SymbolTable
{
    vector<map<string, SymbolEntry>> scopes;

    void enter_scope() { scopes.push_back({}); }
    void exit_scope()  { if (!scopes.empty()) scopes.pop_back(); }
    int  current_level() const { return (int)scopes.size() - 1; }

    bool declare(const string& name, const string& type)
    {
        if (scopes.empty()) scopes.push_back({});
        auto& cur = scopes.back();
        if (cur.count(name)) return false;
        cur[name] = SymbolEntry{name, type, current_level(), ""};
        return true;
    }

    SymbolEntry* lookup(const string& name)
    {
        for (int i = (int)scopes.size() - 1; i >= 0; i--)
        {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) return &it->second;
        }
        return nullptr;
    }
};

struct SemanticResult { ASTNode* ast_root = nullptr; vector<SymbolEntry> symbols; vector<string> errors; };

// 类型助手
inline string promote(const string& a, const string& b)             // 算术类型提升
{
    if (a == "float" || b == "float") return "float";
    if (a == "int"   && b == "int")   return "int";
    return a.empty() ? b : a;
}

inline bool compatible(const string& target, const string& value)   // 赋值相容（含 int→float）
{
    if (target.empty() || value.empty()) return true;
    if (target == value) return true;
    if (target == "float" && value == "int") return true;
    return false;
}

inline string elem_type(const string& t)                            // 去掉数组的 "[]"
{
    if (t.size() >= 2 && t.substr(t.size() - 2) == "[]") return t.substr(0, t.size() - 2);
    return t;
}

// 遍历 AST 做语义分析：作用域 / 声明 / 查找 / 类型检查
struct Analyzer
{
    SymbolTable          sym;
    vector<SymbolEntry>& symbols;
    vector<string>&      errors;
    string               scope_label = "全局";
    string               cur_ret_type;

    Analyzer(vector<SymbolEntry>& s, vector<string>& e) : symbols(s), errors(e) {}

    void declare(const string& name, const string& type)
    {
        if (!sym.declare(name, type))
            errors.push_back("重复声明：" + name + "（当前作用域已有同名符号）");
        else
            symbols.push_back(SymbolEntry{name, type, sym.current_level(), scope_label});
    }

    void walk(ASTNode* nd)
    {
        if (!nd) return;
        switch (nd->node_type)
        {
            case AST_PROGRAM:
                sym.enter_scope();                          // 全局作用域
                for (ASTNode* c : nd->children)             // 预登记顶层函数（支持递归）
                    if (c->node_type == AST_FUN_DECL) declare(c->name, c->val_type);
                for (ASTNode* c : nd->children) walk(c);
                sym.exit_scope();
                break;
            case AST_FUN_DECL:
            {
                string saved_scope = scope_label, saved_ret = cur_ret_type;
                scope_label = nd->name; cur_ret_type = nd->val_type;
                sym.enter_scope();                          // 函数体作用域
                for (ASTNode* c : nd->children)
                {
                    if (c->node_type == AST_PARAM) declare(c->name, c->val_type);
                    else walk(c);
                }
                sym.exit_scope();
                scope_label = saved_scope; cur_ret_type = saved_ret;
                break;
            }
            case AST_VAR_DECL:
                declare(nd->name, nd->val_type);
                if (!nd->children.empty())                  // 初值类型检查
                {
                    walk(nd->children[0]);
                    if (!compatible(nd->val_type, nd->children[0]->val_type))
                        errors.push_back("类型不匹配：用 " + nd->children[0]->val_type +
                                         " 初始化 " + nd->name + "(" + nd->val_type + ")");
                }
                break;
            case AST_ASSIGN:                                // [目标, 右值]
            {
                walk(nd->children[0]); walk(nd->children[1]);
                string lt = nd->children[0]->val_type, rt = nd->children[1]->val_type;
                if (!compatible(lt, rt))
                    errors.push_back("类型不匹配：不能把 " + rt + " 赋给 " +
                                     nd->children[0]->name + "(" + lt + ")");
                break;
            }
            case AST_RETURN_STMT:
            {
                if (!nd->children.empty()) walk(nd->children[0]);
                string rt = nd->children.empty() ? "void" : nd->children[0]->val_type;
                if (cur_ret_type == "void" && rt != "void" && !rt.empty())
                    errors.push_back("返回值类型不符：void 函数不应返回值");
                else if (cur_ret_type != "void" && !compatible(cur_ret_type, rt))
                    errors.push_back("返回值类型不符：函数声明返回 " + cur_ret_type +
                                     "，实际返回 " + rt);
                break;
            }
            case AST_BINOP:                                 // 自底向上算类型
                for (ASTNode* c : nd->children) walk(c);
                if (nd->name=="+"||nd->name=="-"||nd->name=="*"||nd->name=="/")
                    nd->val_type = promote(nd->children[0]->val_type, nd->children[1]->val_type);
                else
                    nd->val_type = "int";
                break;
            case AST_ID:
                if (SymbolEntry* e = sym.lookup(nd->name)) nd->val_type = e->type;
                else errors.push_back("未声明的标识符：" + nd->name);
                break;
            case AST_CALL:
                if (SymbolEntry* e = sym.lookup(nd->name)) nd->val_type = e->type;
                else errors.push_back("未声明的标识符：" + nd->name);
                for (ASTNode* c : nd->children) walk(c);
                break;
            case AST_ARRAY_ACCESS:
                if (SymbolEntry* e = sym.lookup(nd->name)) nd->val_type = elem_type(e->type);
                else errors.push_back("未声明的标识符：" + nd->name);
                for (ASTNode* c : nd->children) walk(c);
                break;
            case AST_COMP_STMT:                             // 复合语句开块作用域
                sym.enter_scope();
                for (ASTNode* c : nd->children) walk(c);
                sym.exit_scope();
                break;
            default:
                for (ASTNode* c : nd->children) walk(c);
                break;
        }
    }
};

inline void analyze(SemanticResult& result)
{
    if (!result.ast_root) return;
    Analyzer az(result.symbols, result.errors);
    az.walk(result.ast_root);
}

// 结果打印
inline void print_ast(ASTNode* node, int depth = 0)
{
    if (!node) return;
    for (int i = 0; i < depth; i++) cout << "  ";
    cout << node_type_name(node->node_type);
    if (!node->name.empty())     cout << " (" << node->name << ")";
    if (!node->val_type.empty()) cout << " : " << node->val_type;
    cout << "\n";
    for (ASTNode* c : node->children) print_ast(c, depth + 1);
}

inline void print_symbol_table(const vector<SymbolEntry>& symbols)
{
    if (symbols.empty()) { cout << "(空)" << endl; return; }
    string cur = "\x01";
    for (auto& s : symbols)
    {
        if (s.scope != cur) { cur = s.scope; cout << "作用域 [" << cur << "]:" << endl; }
        cout << "  " << s.name << " : " << s.type << "  (层级 " << s.scope_level << ")" << endl;
    }
}

inline void print_errors(const vector<string>& errors)
{
    if (errors.empty()) { cout << "无错误" << endl; return; }
    cout << "共 " << errors.size() << " 条:" << endl;
    for (auto& e : errors) cout << "  " << e << endl;
}
