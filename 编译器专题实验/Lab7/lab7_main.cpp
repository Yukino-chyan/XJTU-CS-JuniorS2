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
