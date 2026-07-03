// Lab5 语义分析：读入实验四导出的 SLR(1) 分析表和 Lab2 的 token 流，
// 解析建 AST、建符号表、做类型检查。数据结构与字符串工具见 slr1_base.h。

#include "slr1_base.h"
#include <memory>
#include <stack>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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

// 读取实验四导出的 SLR(1) 分析表（产生式 + ACTION + GOTO）
SLR1Result read_slr1_table(const string& path)
{
    SLR1Result r;
    ifstream fin(path);
    if (!fin.is_open())
    {
        cerr << "无法打开分析表文件: " << path << endl;
        return r;
    }
    string line, mode;
    while (getline(fin, line))
    {
        line = trim(line);
        if (line.empty()) continue;
        istringstream in(line);
        string head; in >> head;

        if (head == "STATES")
        {
            int n; in >> n;
            r.table.action.assign(n, {});
            r.table.go_to.assign(n, {});
            mode = "";
        }
        else if (head == "PRODUCTIONS")
        {
            int n; in >> n;
            r.productions.resize(n);
            mode = "P";
        }
        else if (head == "TERMINALS")
        {
            string s; while (in >> s) r.table.terminals.push_back(s);
            mode = "";
        }
        else if (head == "NONTERMINALS")
        {
            string s; while (in >> s) r.table.nonterminals.push_back(s);
            mode = "";
        }
        else if (head == "ACTION") { mode = "A"; }
        else if (head == "GOTO")   { mode = "G"; }
        else if (mode == "P")               // 产生式行：编号 左部 右部...
        {
            int idx = stoi(head);
            Production p; in >> p.left;
            string s; while (in >> s) p.right.push_back(s);
            if (idx >= 0 && idx < (int)r.productions.size()) r.productions[idx] = p;
        }
        else if (mode == "A")               // ACTION 行：状态 终结符 动作
        {
            int st = stoi(head); string term, act; in >> term >> act;
            Action a{0, 0};
            if (act == "acc")                       { a.type = 3; a.value = 0; }
            else if (!act.empty() && act[0] == 's') { a.type = 1; a.value = stoi(act.substr(1)); }
            else if (!act.empty() && act[0] == 'r') { a.type = 2; a.value = stoi(act.substr(1)); }
            if (st >= 0 && st < (int)r.table.action.size()) r.table.action[st][term] = a;
        }
        else if (mode == "G")               // GOTO 行：状态 非终结符 目标
        {
            int st = stoi(head); string nt; int tgt; in >> nt >> tgt;
            if (st >= 0 && st < (int)r.table.go_to.size()) r.table.go_to[st][nt] = tgt;
        }
    }
    return r;
}

// 解析 Lab2 词法输出每行 "(TYPE, lexeme)"（按第一个逗号切分，兼容词素为逗号）
vector<TokenRecord> read_tokens(const string& path)
{
    vector<TokenRecord> tokens;
    ifstream fin(path);
    if (!fin.is_open())
    {
        cerr << "无法打开 Token 文件: " << path << endl;
        return tokens;
    }
    string line;
    while (getline(fin, line))
    {
        line = trim(line);
        if (line.empty()) continue;
        if (line.front() == '(' && line.back() == ')')
            line = line.substr(1, line.size() - 2);
        size_t comma = line.find(',');
        if (comma == string::npos) continue;
        TokenRecord t;
        t.type   = trim(line.substr(0, comma));
        t.lexeme = trim(line.substr(comma + 1));
        if (!t.type.empty()) tokens.push_back(t);
    }
    return tokens;
}

// token 类型名 → 文法终结符名（两者一致，恒等映射）
string token_to_terminal(const string& type)
{
    return type;
}

// 终结符名 → 可读符号（让语法错误提示能讲人话）
static string term_readable(const string& t)
{
    static const map<string, string> M = {
        {"SCO",";"},{"CMA",","},{"LPA","("},{"RPA",")"},{"LBK","["},{"RBK","]"},
        {"LBR","{"},{"RBR","}"},{"ASG","="},{"ADD","+"},{"SUB","-"},{"MUL","*"},{"DIV","/"},
        {"LT","<"},{"LE","<="},{"EQ","=="},{"GT",">"},{"GE",">="},{"NE","!="},
        {"INT","int"},{"FLOAT","float"},{"VOID","void"},{"IF","if"},{"ELSE","else"},
        {"WHILE","while"},{"RETURN","return"},{"INPUT","input"},{"PRINT","print"},
        {"ID","标识符"},{"NUM","整数"},{"FLO","浮点数"},{"$","文件结束"}
    };
    auto it = M.find(t);
    return it == M.end() ? t : it->second;
}

// ---- 符号表实现 ----
void SymbolTable::enter_scope() { scopes.push_back({}); }        // 压入一层
void SymbolTable::exit_scope()  { if (!scopes.empty()) scopes.pop_back(); }

bool SymbolTable::declare(const string& name, const string& type)   // 当前层重复则失败
{
    if (scopes.empty()) scopes.push_back({});
    auto& cur = scopes.back();
    if (cur.count(name)) return false;
    cur[name] = SymbolEntry{name, type, current_level(), ""};
    return true;
}

SymbolEntry* SymbolTable::lookup(const string& name)             // 由内向外查找
{
    for (int i = (int)scopes.size() - 1; i >= 0; i--)
    {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return &it->second;
    }
    return nullptr;
}

// ---- 类型助手 ----
static string promote(const string& a, const string& b)         // 算术类型提升
{
    if (a == "float" || b == "float") return "float";
    if (a == "int"   && b == "int")   return "int";
    return a.empty() ? b : a;
}

static bool compatible(const string& target, const string& value)   // 赋值相容（含 int→float）
{
    if (target.empty() || value.empty()) return true;
    if (target == value) return true;
    if (target == "float" && value == "int") return true;
    return false;
}

static string elem_type(const string& t)                        // 去掉数组的 "[]"
{
    if (t.size() >= 2 && t.substr(t.size() - 2) == "[]") return t.substr(0, t.size() - 2);
    return t;
}

// 遍历 AST 做语义分析：作用域 / 声明 / 查找 / 类型检查
namespace
{
struct Analyzer
{
    SymbolTable          sym;
    vector<SymbolEntry>& symbols;   // 已声明符号（展示用）
    vector<string>&      errors;
    string               scope_label = "全局";
    string               cur_ret_type;     // 当前函数返回类型

    Analyzer(vector<SymbolEntry>& s, vector<string>& e) : symbols(s), errors(e) {}

    void declare(const string& name, const string& type)
    {
        if (!sym.declare(name, type))
            errors.push_back("重复声明：" + name + "（当前作用域已有同名符号）");
        else
            symbols.push_back(SymbolEntry{name, type, sym.current_level(), scope_label});
    }

    void walk(const ASTPtr& nd)
    {
        if (!nd) return;
        switch (nd->node_type)
        {
            case AST_PROGRAM:
                sym.enter_scope();                          // 全局作用域
                for (auto& c : nd->children)                // 预登记顶层函数（支持递归）
                    if (c->node_type == AST_FUN_DECL) declare(c->name, c->val_type);
                for (auto& c : nd->children) walk(c);
                sym.exit_scope();
                break;
            case AST_FUN_DECL:
            {
                string saved_scope = scope_label, saved_ret = cur_ret_type;
                scope_label = nd->name; cur_ret_type = nd->val_type;
                sym.enter_scope();                          // 函数体作用域
                for (auto& c : nd->children)
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
                for (auto& c : nd->children) walk(c);
                if (nd->name=="+"||nd->name=="-"||nd->name=="*"||nd->name=="/")
                    nd->val_type = promote(nd->children[0]->val_type, nd->children[1]->val_type);
                else
                    nd->val_type = "int";                   // 关系运算（布尔）
                break;
            case AST_ID:
                if (SymbolEntry* e = sym.lookup(nd->name)) nd->val_type = e->type;
                else errors.push_back("未声明的标识符：" + nd->name);
                break;
            case AST_CALL:
                if (SymbolEntry* e = sym.lookup(nd->name)) nd->val_type = e->type;
                else errors.push_back("未声明的标识符：" + nd->name);
                for (auto& c : nd->children) walk(c);
                break;
            case AST_ARRAY_ACCESS:
                if (SymbolEntry* e = sym.lookup(nd->name)) nd->val_type = elem_type(e->type);
                else errors.push_back("未声明的标识符：" + nd->name);
                for (auto& c : nd->children) walk(c);
                break;
            case AST_COMP_STMT:                             // 复合语句开块作用域
                sym.enter_scope();
                for (auto& c : nd->children) walk(c);
                sym.exit_scope();
                break;
            default:
                for (auto& c : nd->children) walk(c);
                break;
        }
    }
};
} // namespace

void analyze(SemanticResult& result)
{
    if (!result.ast_root) return;
    Analyzer az(result.symbols, result.errors);
    az.walk(result.ast_root);
}

// 把 list 容器节点的子节点摊平到 dst
static void splice_children(const ASTPtr& dst, const ASTPtr& list)
{
    if (list) for (auto& c : list->children) dst->children.push_back(c);
}

// 语义动作：按产生式归约时建对应的 AST 节点（$$ = res，$i = vals[i-1]）
SemanticValue semantic_action(int prod_id,
        const vector<SemanticValue>& vals,
        const vector<Production>& productions)
{
    const Production& p = productions[prod_id];
    const string& L = p.left;
    const auto&   R = p.right;
    int n = (int)R.size();
    SemanticValue res;

    if (L == "Prog")                                    // Prog -> DeclList StmtList
    {
        auto node = make_node(AST_PROGRAM);
        splice_children(node, vals[0].ast);             // 各声明
        splice_children(node, vals[1].ast);             // 入口语句
        res.ast = node;
    }
    else if (L == "DeclList")
    {
        if (n == 0) res.ast = make_node(AST_DECL_LIST); // ε
        else                                            // DeclList Decl SCO
        {
            res.ast = vals[0].ast;
            if (vals[1].ast) res.ast->children.push_back(vals[1].ast);
        }
    }
    else if (L == "Decl")
    {
        string type = vals[0].type;
        string name = vals[1].lexeme;
        if (n == 2)                                     // Type ID
            res.ast = make_node(AST_VAR_DECL, name, type);
        else if (R[2] == "ASG")                         // Type ID ASG Expr
            res.ast = make_node(AST_VAR_DECL, {vals[3].ast}, name, type);
        else if (R[2] == "LBK")                         // Type ID LBK NUM RBK（数组）
            res.ast = make_node(AST_VAR_DECL, name, type + "[]");
        else                                            // 函数声明
        {
            auto node = make_node(AST_FUN_DECL, name, type);
            splice_children(node, vals[3].ast);         // 形参
            splice_children(node, vals[6].ast);         // 函数体声明
            splice_children(node, vals[7].ast);         // 函数体语句
            res.ast = node;
        }
    }
    else if (L == "Type")                               // INT/FLOAT/VOID
    {
        res.type = vals[0].lexeme; res.lexeme = vals[0].lexeme;
        res.ast  = make_node(AST_TYPE, vals[0].lexeme);
    }
    else if (L == "ParamList")
    {
        if (n == 0) res.ast = make_node(AST_PARAM_LIST);    // ε
        else                                                // ParamList Param SCO
        {
            res.ast = vals[0].ast;
            if (vals[1].ast) res.ast->children.push_back(vals[1].ast);
        }
    }
    else if (L == "Param")
    {
        string type = vals[0].type + (n == 4 ? "[]" : "");  // Type ID [LBK RBK]
        res.ast = make_node(AST_PARAM, vals[1].lexeme, type);
    }
    else if (L == "StmtList")
    {
        if (n == 1)                                     // StmtList -> Stmt
        {
            res.ast = make_node(AST_STMT_LIST);
            if (vals[0].ast) res.ast->children.push_back(vals[0].ast);
        }
        else if (n == 3)                                // StmtList SCO Stmt
        {
            res.ast = vals[0].ast;
            if (vals[2].ast) res.ast->children.push_back(vals[2].ast);
        }
        else res.ast = vals[0].ast;                     // StmtList SCO（尾随分号）
    }
    else if (L == "Stmt")
    {
        if (n == 3 && R[1] == "ASG")                    // ID ASG Expr
        {
            auto target = make_node(AST_ID, vals[0].lexeme);
            res.ast = make_node(AST_ASSIGN, {target, vals[2].ast});
        }
        else if (R[1] == "LBK")                         // ID LBK Expr RBK ASG Expr
        {
            auto target = make_node(AST_ARRAY_ACCESS, {vals[2].ast}, vals[0].lexeme);
            res.ast = make_node(AST_ASSIGN, {target, vals[5].ast});
        }
        else if (R[0] == "IF" && n == 5)                // if
            res.ast = make_node(AST_IF_STMT, {vals[2].ast, vals[4].ast});
        else if (R[0] == "IF")                          // if-else
            res.ast = make_node(AST_IF_STMT, {vals[2].ast, vals[4].ast, vals[6].ast});
        else if (R[0] == "WHILE")                       // while
            res.ast = make_node(AST_WHILE_STMT, {vals[2].ast, vals[4].ast});
        else if (R[0] == "RETURN")                      // return
        {
            res.ast = make_node(AST_RETURN_STMT, {vals[1].ast});
            res.ast->val_type = vals[1].type;
        }
        else if (R[0] == "LBR")                         // 复合语句
        {
            auto node = make_node(AST_COMP_STMT);
            splice_children(node, vals[1].ast);
            res.ast = node;
        }
        else if (R[0] == "ID" && R[1] == "LPA")         // 调用语句
        {
            auto node = make_node(AST_CALL, vals[0].lexeme);
            splice_children(node, vals[2].ast);
            res.ast = node;
        }
        else if (R[0] == "PRINT")                       // print Expr（无括号；print(a) 中 (a) 也是 Expr）
            res.ast = make_node(AST_PRINT_STMT, {vals[1].ast});
        else if (R[0] == "INPUT")                       // input ID
            res.ast = make_node(AST_INPUT_STMT, {make_node(AST_ID, vals[1].lexeme)});
    }
    else if (L == "Bool")
    {
        if (n == 3 && R[1] == "ASG")                    // ID ASG Expr：赋值作条件，条件值为所赋的值
        {
            auto target = make_node(AST_ID, vals[0].lexeme);
            res.ast  = make_node(AST_ASSIGN, {target, vals[2].ast});
            res.type = vals[2].type;
        }
        else if (n == 3)                                // Expr RelOp Expr
        {
            res.ast = make_node(AST_BINOP, {vals[0].ast, vals[2].ast}, vals[1].lexeme, "int");
            res.type = "int";
        }
        else return vals[0];                            // Expr
    }
    else if (L == "RelOp") { res.lexeme = vals[0].lexeme; }   // 上传关系运算符
    else if (L == "Expr")
    {
        if (n == 1) return vals[0];                     // Expr -> Term
        string t = promote(vals[0].type, vals[2].type); // Expr ADD/SUB Term
        res.ast  = make_node(AST_BINOP, {vals[0].ast, vals[2].ast}, vals[1].lexeme, t);
        res.type = t;
    }
    else if (L == "Term")
    {
        if (n == 1) return vals[0];                     // Term -> Fact
        string t = promote(vals[0].type, vals[2].type); // Term MUL/DIV Fact
        res.ast  = make_node(AST_BINOP, {vals[0].ast, vals[2].ast}, vals[1].lexeme, t);
        res.type = t;
    }
    else if (L == "Fact")
    {
        if (n == 1) return vals[0];                     // NUM / FLO / ID
        if (R[0] == "LPA") return vals[1];              // LPA Expr RPA
        if (R[1] == "LBK")                              // 数组访问
            res.ast = make_node(AST_ARRAY_ACCESS, {vals[2].ast}, vals[0].lexeme);
        else                                            // 函数调用
        {
            auto node = make_node(AST_CALL, vals[0].lexeme);
            splice_children(node, vals[2].ast);
            res.ast = node;
        }
    }
    else if (L == "ArgList")
    {
        if (n == 0) res.ast = make_node(AST_ARG_LIST);  // ε
        else                                            // ArgList Arg CMA
        {
            res.ast = vals[0].ast;
            if (vals[1].ast) res.ast->children.push_back(vals[1].ast);
        }
    }
    else if (L == "Arg")
    {
        if (n == 1) return vals[0];                     // Expr
        res.ast = make_node(AST_ARRAY_ACCESS, vals[0].lexeme);   // ID LBK RBK
    }
    else if (!vals.empty()) return vals[0];             // 增广起始等，直接上传

    return res;
}

// 移进终结符时构造叶节点（标点/运算符不建节点）
static SemanticValue make_leaf(const string& term, const string& lexeme)
{
    SemanticValue v;
    v.lexeme = lexeme;
    if (term == "ID")        v.ast = make_node(AST_ID, lexeme);
    else if (term == "NUM") { v.ast = make_node(AST_NUM, lexeme, "int");   v.type = "int"; }
    else if (term == "FLO") { v.ast = make_node(AST_FLO, lexeme, "float"); v.type = "float"; }
    else if (term == "INT" || term == "FLOAT" || term == "VOID")
                             v.ast = make_node(AST_TYPE, lexeme);
    else                     v.ast = nullptr;
    return v;
}

// SLR(1) 解析驱动：状态栈 + 属性栈，移进/归约/接受/出错
SemanticResult run_semantic(const SLR1Result& slr, const vector<TokenRecord>& tokens)
{
    SemanticResult result;
    const auto& action = slr.table.action;
    const auto& go_to  = slr.table.go_to;
    const auto& prods  = slr.productions;

    vector<int>           state_stack{0};   // 状态栈
    vector<SemanticValue> attr_stack;       // 属性栈
    size_t ip = 0;                          // 输入指针（越界视为 $）

    while (true)
    {
        int s = state_stack.back();
        string a      = (ip < tokens.size()) ? token_to_terminal(tokens[ip].type) : "$";
        string lexeme = (ip < tokens.size()) ? tokens[ip].lexeme                   : "";

        const auto& row = action[s];
        auto it = row.find(a);
        if (it == row.end())                         // 空表项 → 报错，并解释原因
        {
            // 该状态下 ACTION 表里有项的终结符 = 此刻“期望”的下一个 token
            set<string> expset;
            bool wantSemicolon = false;
            for (const auto& kv : row)
            {
                expset.insert(term_readable(kv.first));
                if (kv.first == "SCO") wantSemicolon = true;
            }
            string expected;
            for (const auto& e : expset)
                expected += (expected.empty() ? "" : " ") + ("\"" + e + "\"");

            string msg;
            if (a == "ERROR")                        // 扫描器没认出的字符 → 词法错误
                msg = "词法错误：无法识别的字符 \"" + lexeme +
                      "\"（标识符只允许 a-z 与 0-9，不支持下划线/全角符号/引号）";
            else if (a == "$")                       // 输入提前结束
                msg = "语法错误：程序意外结束，此处期望 " + expected +
                      "（可能缺少 \"}\" 或 \";\"）";
            else                                     // 出现了不该出现的 token
                msg = "语法错误：意外的 " + term_readable(a) + " \"" + lexeme +
                      "\"，此处期望 " + expected +
                      (wantSemicolon ? "（可能缺少分号 \";\"）" : "");

            result.errors.push_back(msg);
            break;
        }

        const Action& act = it->second;
        if (act.type == 1)                           // 移进
        {
            state_stack.push_back(act.value);
            attr_stack.push_back(make_leaf(a, lexeme));
            ip++;
        }
        else if (act.type == 2)                      // 归约
        {
            const Production& p = prods[act.value];
            int n = (int)p.right.size();
            vector<SemanticValue> vals(attr_stack.end() - n, attr_stack.end());
            for (int i = 0; i < n; i++) { attr_stack.pop_back(); state_stack.pop_back(); }

            SemanticValue nv = semantic_action(act.value, vals, prods);
            attr_stack.push_back(nv);

            int t = state_stack.back();              // 归约后查 GOTO
            auto git = go_to[t].find(p.left);
            if (git == go_to[t].end())
            {
                result.errors.push_back("GOTO 缺失：状态 " + to_string(t) +
                                        " 非终结符 " + p.left);
                break;
            }
            state_stack.push_back(git->second);
        }
        else if (act.type == 3)                      // 接受
        {
            if (!attr_stack.empty()) result.ast_root = attr_stack.back().ast;
            break;
        }
        else
        {
            result.errors.push_back("未知动作类型");
            break;
        }
    }
    return result;
}

// 递归缩进打印 AST
void print_ast(const ASTPtr& node, int depth = 0)
{
    if (!node) return;
    for (int i = 0; i < depth; i++) cout << "  ";
    if (node->node_type == AST_EMPTY && !node->name.empty())
    {
        cout << node->name;
    }
    else
    {
        cout << node_type_name(node->node_type);
        if (!node->name.empty()) cout << " (" << node->name << ")";
    }
    if (!node->val_type.empty()) cout << " : " << node->val_type;
    cout << "\n";
    for (auto& c : node->children) print_ast(c, depth + 1);
}

// 按作用域分组打印符号表
void print_symbol_table(const vector<SymbolEntry>& symbols)
{
    if (symbols.empty()) { cout << "(空)" << endl; return; }
    string cur = "\x01";   // 分组哨兵
    for (auto& s : symbols)
    {
        if (s.scope != cur) { cur = s.scope; cout << "作用域 [" << cur << "]:" << endl; }
        cout << "  " << pad(s.name, 12) << " : " << s.type
             << "  (层级 " << s.scope_level << ")" << endl;
    }
}

void print_errors(const vector<string>& errors)
{
    if (errors.empty()) { cout << "无错误" << endl; return; }
    cout << "共 " << errors.size() << " 条:" << endl;
    for (auto& e : errors) cout << "  " << e << endl;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    string table_path = "slr1_table.txt";   // 实验四导出的分析表
    string token_path = "../Lab2/out.txt";  // Lab2 词法输出
    if (argc >= 2) table_path = argv[1];
    if (argc >= 3) token_path = argv[2];

    auto slr = read_slr1_table(table_path);          // 读分析表
    cout << "=== SLR(1) 分析表 (" << slr.table.action.size() << " 状态, "
         << slr.productions.size() << " 产生式) ===" << endl << endl;

    auto tokens = read_tokens(token_path);           // 读 token 流
    cout << "=== Token 流 (" << tokens.size() << " 个) ===" << endl << endl;

    auto result = run_semantic(slr, tokens);         // 解析 + 建 AST
    analyze(result);                                 // 语义分析

    cout << "=== AST ===" << endl;
    print_ast(result.ast_root);
    cout << endl << "=== 符号表 ===" << endl;
    print_symbol_table(result.symbols);
    cout << endl << "=== 语义检查 ===" << endl;
    print_errors(result.errors);

    return 0;
}
