#include <fstream>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "frontend.h"
#include "irgen.h"

// 读取整个源文件到字符串
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
        cerr << "用法: " << argv[0] << " <源程序.src> [分析表=slr1_table.txt]" << endl;
        return 1;
    }
    string src_path   = argv[1];
    string table_path = (argc >= 3) ? argv[2] : "slr1_table.txt";

    // 词法
    string src = read_file(src_path);
    vector<TokenRecord> tokens = tokenize(src);

    cout << "=== Token 序列 ===" << endl;
    for (size_t i = 0; i < tokens.size(); i++)
        cout << "  " << (i + 1) << ". <" << tokens[i].type
             << ", \"" << tokens[i].lexeme << "\">" << endl;

    // 解析 + 语义（读分析表 → 建 AST + 符号表 + 类型检查）
    SLR1Result slr = read_slr1_table(table_path);
    SemanticResult result = parse_and_analyze(slr, tokens);

    cout << endl << "=== AST ===" << endl;
    print_ast(result.ast_root, 0);
    cout << endl << "=== 符号表 ===" << endl;
    print_symbol_table(result.symbols);
    cout << endl << "=== 语义检查 ===" << endl;
    print_errors(result.errors);

    // 中间代码生成
    if (!result.errors.empty())
    {
        cout << endl << "[存在词法/语法/语义错误，暂不生成中间代码]" << endl;
        return 0;
    }
    cout << endl << "=== 四元式 ===" << endl;
    vector<Quad> quads = generate_ir(result.ast_root);
    if (quads.empty())
        cout << "（暂无四元式：翻译逻辑 Task2-5 待实现）" << endl;
    else
        print_quads(quads, cout);

    return 0;
}
