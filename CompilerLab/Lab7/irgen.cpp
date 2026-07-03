// Lab6 中间代码生成 —— Task1：四元式基础设施（数据结构 / 临时变量 / 跳转回填 / 输出）。
// 具体的 AST → 四元式翻译（表达式、赋值、控制流）在 Task2-5 的 IRGen::gen 中实现。

#include "irgen.h"

// 生成新临时变量：T1, T2, T3, ...
string IRGen::newTemp()
{
    return "T" + to_string(++tempCount);
}

// 下一条将要生成的四元式编号（1 基，与打印编号一致）
int IRGen::nextQuad() const
{
    return (int)code.size() + 1;
}

// 追加一条四元式，返回它的编号（1 基）
int IRGen::emit(const string& op, const string& a1,
                const string& a2, const string& res)
{
    code.push_back(Quad{op, a1, a2, res});
    return (int)code.size();
}

// 把第 quadNo 条四元式的 result 回填为目标编号（用于 if/while 的跳转目标）
void IRGen::backpatch(int quadNo, int target)
{
    if (quadNo >= 1 && quadNo <= (int)code.size())
        code[quadNo - 1].result = to_string(target);
}

// ── Task2：表达式翻译 ──
// 输入一个表达式节点，发射其四元式，返回它的 place（值最终所在：常量/变量/临时变量）
string IRGen::genExpr(const ASTPtr& e)
{
    if (!e) return "";
    switch (e->node_type)
    {
        case AST_NUM:
        case AST_FLO:
            return e->name;                       // 字面量：place 就是它本身
        case AST_ID:
            return e->name;                       // 变量：place 就是变量名

        case AST_BINOP:                           // E op E：算出两边，再发一条
        {
            string l = genExpr(e->children[0]);
            string r = genExpr(e->children[1]);
            string t = newTemp();
            emit(e->name, l, r, t);               // (op, l, r, t)，op 取自节点（+ - * /）
            return t;
        }

        case AST_ARRAY_ACCESS:                    // d[E]：读数组元素
        {
            if (e->children.empty()) return e->name;   // d[]（按名传数组）：place = 数组名
            string idx = genExpr(e->children[0]);
            string t = newTemp();
            emit("=[]", e->name, idx, t);         // t = d[idx]
            return t;
        }

        case AST_CALL:                            // d(R)：函数调用，取返回值
        {
            int argc = 0;
            for (auto& a : e->children) { emit("param", genExpr(a), "", ""); argc++; }
            string t = newTemp();
            emit("call", e->name, to_string(argc), t);  // t = call d, argc
            return t;
        }

        default:
            return "";                            // 其它节点不是表达式
    }
}

// ── Task4：条件跳转辅助 ──
// 翻译布尔条件，发射一条"条件为假时跳转"的四元式，返回其编号供回填。
// 若条件是关系表达式（如 a < b），直接对关系取反发跳转，少一条临时变量指令；
// 否则把条件当普通值翻译，用 jz（值为零即假时跳）。
int IRGen::genCondFalseJump(const ASTPtr& cond)
{
    if (cond && cond->node_type == AST_BINOP)
    {
        static const map<string,string> neg = {
            {"<","j>="}, {"<=","j>"}, {">","j<="}, {">=","j<"}, {"==","j!="}, {"!=","j=="}
        };
        auto it = neg.find(cond->name);
        if (it != neg.end())
        {
            string l = genExpr(cond->children[0]);
            string r = genExpr(cond->children[1]);
            return emit(it->second, l, r, "");   // 取反跳转，目标待回填
        }
    }
    string place = genExpr(cond);
    return emit("jz", place, "", "");            // 值为假（零）时跳
}

// ── Task4：控制流翻译 ──
// if 语句：有/无 else 两种情况
// while 语句：循环体末尾无条件跳回条件入口

// 遍历入口
void IRGen::gen(const ASTPtr& n)
{
    if (!n) return;
    switch (n->node_type)
    {
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

        case AST_ASSIGN:
        {
            const ASTPtr& target = n->children[0];
            string rhs = genExpr(n->children[1]);
            if (target->node_type == AST_ID)
                emit("=", rhs, "", target->name);          // (=, rhs, , d)
            else if (target->node_type == AST_ARRAY_ACCESS)
            {
                string idx = genExpr(target->children[0]);
                emit("[]=", rhs, idx, target->name);       // ([]=, rhs, idx, d)
            }
            break;
        }

        case AST_IF_STMT:
        {
            bool has_else = (n->children.size() >= 3);
            int q_false = genCondFalseJump(n->children[0]);   // 条件假时跳走
            gen(n->children[1]);                               // then 分支
            if (has_else)
            {
                int q_skip = emit("j", "", "", "");            // then 结束后跳过 else
                backpatch(q_false, nextQuad());                // 假跳到 else 入口
                gen(n->children[2]);                           // else 分支
                backpatch(q_skip, nextQuad());                 // skip 跳到 else 之后
            }
            else
            {
                backpatch(q_false, nextQuad());                // 假跳到 if 之后
            }
            break;
        }

        case AST_WHILE_STMT:
        {
            int entry = nextQuad();                            // 条件判断入口，循环末尾要跳回这里
            int q_false = genCondFalseJump(n->children[0]);   // 条件假时跳出循环
            gen(n->children[1]);                               // 循环体
            emit("j", "", "", to_string(entry));               // 无条件跳回条件入口
            backpatch(q_false, nextQuad());                    // 假跳到循环之后
            break;
        }

        // ── Task5：return / print / input / 独立函数调用 ──

        case AST_RETURN_STMT:
        {
            string val = n->children.empty() ? "" : genExpr(n->children[0]);
            emit("return", val, "", "");
            break;
        }

        case AST_PRINT_STMT:
        {
            string val = genExpr(n->children[0]);
            emit("print", val, "", "");
            break;
        }

        case AST_INPUT_STMT:
        {
            string id = n->children[0]->name;   // children[0] 是 AST_ID 节点
            emit("input", "", "", id);
            break;
        }

        case AST_CALL:        // 独立函数调用语句（如末尾 main()），返回值丢弃
        case AST_EXPR_STMT:
            genExpr(n);       // 复用 genExpr：发 param + call，返回值不使用
            break;

        default:
            break;   // 声明/形参/类型等不产生四元式
    }
}

vector<Quad> generate_ir(const ASTPtr& root)
{
    IRGen g;
    g.gen(root);
    return g.code;
}

// 按 "编号. (op, arg1, arg2, result)" 输出；空字段留空，对齐 PPT 验收示例
void print_quads(const vector<Quad>& code, ostream& os)
{
    for (size_t i = 0; i < code.size(); i++)
    {
        const Quad& q = code[i];
        os << (i + 1) << ". (" << q.op << ", " << q.arg1
           << ", " << q.arg2 << ", " << q.result << ")\n";
    }
}

// ── Task1 自测：单独编译 (-DIRGEN_SELFTEST) 验证基础设施与输出格式 ──
#ifdef IRGEN_SELFTEST
int main()
{
    IRGen g;
    // 示例 X = a*b + c/d（对照 PPT 四元式）
    string t1 = g.newTemp(); g.emit("*", "a", "b", t1);   // (*, a, b, T1)
    string t2 = g.newTemp(); g.emit("/", "c", "d", t2);   // (/, c, d, T2)
    string t3 = g.newTemp(); g.emit("+", t1, t2, t3);     // (+, T1, T2, T3)
    g.emit("=", t3, "", "X");                             // (=, T3, , X)

    // 演示跳转回填：先发目标待定的条件跳转，记录编号，稍后回填到“下一条”
    int j = g.emit("j<", "a", "b", "");
    g.emit("=", "0", "", "X");
    g.backpatch(j, g.nextQuad());                         // 把 j< 的目标填成下一条编号
    g.emit("=", "1", "", "X");

    print_quads(g.code, cout);
    return 0;
}
#endif
