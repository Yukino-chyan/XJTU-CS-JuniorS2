#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// 产生式
struct Production
{
    string left;
    vector<string> right;
};

// LR(0)项目
struct Item
{
    int production_index;
    int dot_pos;
};

// 项目集
struct ItemSet
{
    int id;
    vector<Item> items;
};

// 项目集之间的Goto转移
struct Transition
{
    int from;
    string symbol;
    int to;
};

// 文法
struct Grammar
{
    string start_symbol;
    string augmented_start_symbol;
    vector<Production> productions;
    set<string> nonterminals;
    set<string> terminals;
};

// ACTION表中一个表项的动作
struct Action
{
    // 动作类型：0=报错(空)、1=移进(shift)、2=归约(reduce)、3=接受(accept)
    int type;
    // 移进时为目标状态号；归约时为产生式编号；其它情况未使用
    int value;
};

// SLR(1)分析表
struct SLR1Table
{
    vector<string> terminals;            // ACTION表列头：所有终结符，外加输入结束符 $
    vector<string> nonterminals;         // GOTO表列头：所有非终结符（不含增广开始符号 S'）
    vector<map<string, Action>> action;  // action[状态号][终结符] = 动作
    vector<map<string, int>> go_to;      // go_to[状态号][非终结符] = 目标状态号
    vector<string> conflicts;            // 用 FOLLOW 集后仍无法消解的冲突描述
};

// SLR(1)构造的总结果
struct SLR1Result
{
    Grammar grammar;
    vector<ItemSet> item_sets;
    vector<Transition> transitions;
    vector<string> lr0_conflicts;            // LR(0)层面的冲突（作为引入SLR的动机展示）
    map<string, set<string>> first;          // 各文法符号的FIRST集
    map<string, set<string>> follow;         // 各非终结符的FOLLOW集
    SLR1Table table;                         // 最终的SLR(1)分析表
};

// ════════════════════════════════════════════════════════════════
//  工具函数（沿用实验三）
// ════════════════════════════════════════════════════════════════

// 去掉字符串两端空白
string trim(const string &s)
{
    const string blank = " \t\r\n";
    size_t first = s.find_first_not_of(blank);
    if (first == string::npos) return "";
    size_t last = s.find_last_not_of(blank);
    return s.substr(first, last - first + 1);
}

// 按空格切分右部符号
vector<string> split_symbols(const string &s)
{
    vector<string> res;
    string token;
    istringstream in(s);

    while (in >> token)
        res.push_back(token);

    return res;
}

// 从文件中读取文法，忽略空行和#注释
vector<string> read_grammar_lines(const string &path)
{
    ifstream fin(path);
    vector<string> lines;
    string line;

    while (getline(fin, line))
    {
        size_t p = line.find('#');
        if (p != string::npos)
            line = line.substr(0, p);

        line = trim(line);
        if (line.empty())
            continue;

        // 续行：以 | 开头的行接到上一条产生式后面（支持多行书写产生式）
        if (line[0] == '|' && !lines.empty())
            lines.back() += " " + line;
        else
            lines.push_back(line);
    }

    return lines;
}

// 把字符串右侧补空格到指定宽度，用于对齐分析表的列（表格内容均为ASCII，按字符数补齐即可）
// 计算字符串的终端显示列数（UTF-8：CJK 字符占 2 列，ASCII 占 1 列）
int display_width(const string &s)
{
    int w = 0;
    for (int i = 0; i < (int)s.size(); )
    {
        unsigned char c = s[i];
        if (c < 0x80)       { w += 1; i += 1; }
        else if (c < 0xE0)  { w += 1; i += 2; }
        else if (c < 0xF0)  { w += 2; i += 3; } // CJK 三字节序列，显示宽度 2
        else                { w += 2; i += 4; }
    }
    return w;
}

string pad(const string &s, int width)
{
    string res = s;
    int dw = display_width(s);
    for (int i = dw; i < width; i++) res += " ";
    return res;
}

//  文法解析与 LR(0) 构造（沿用实验三，完整可用）

// 解析文法产生式
Grammar parse_grammar(const vector<string> &lines)
{
    Grammar grammar;
    vector<string> lefts;
    vector<string> rights;
    // 按"->"切分产生式左部和右部，并收集非终结符
    for (int i = 0; i < (int)lines.size(); i++)
    {
        size_t p = lines[i].find("->");
        string left = trim(lines[i].substr(0, p));
        string right = trim(lines[i].substr(p + 2));
        if (i == 0)
            grammar.start_symbol = left;
        grammar.nonterminals.insert(left);
        lefts.push_back(left);
        rights.push_back(right);
    }
    // 按"|"切分候选式，每个候选式保存为一条产生式
    for (int i = 0; i < (int)lefts.size(); i++)
    {
        string right = rights[i];
        size_t start = 0;
        while (true)
        {
            size_t p = right.find('|', start);
            string candidate = (p == string::npos)
                ? trim(right.substr(start))
                : trim(right.substr(start, p - start));

            Production prod;
            prod.left = lefts[i];
            vector<string> syms = split_symbols(candidate);
            // 识别 ε 产生式（grammar.txt 里写 e 或 UTF-8 的 ε 均可）
            // ε 产生式存为空 right，build_action_table 的 dot_pos>=right.size() 天然兼容
            if (syms.size() == 1 && (syms[0] == "e" || syms[0] == u8"ε"))
                prod.right = {};
            else
                prod.right = syms;
            grammar.productions.push_back(prod);
            if (p == string::npos) break;
            start = p + 1;
        }
    }
    // 右部中不属于非终结符的符号就是终结符
    for (int i = 0; i < (int)grammar.productions.size(); i++)
    {
        for (int j = 0; j < (int)grammar.productions[i].right.size(); j++)
        {
            string symbol = grammar.productions[i].right[j];
            if (grammar.nonterminals.count(symbol) == 0)
                grammar.terminals.insert(symbol);
        }
    }
    grammar.augmented_start_symbol = grammar.start_symbol + "'";
    return grammar;
}

// 增广文法：添加新的起始产生式 S' -> S
Grammar augment_grammar(const Grammar &grammar)
{
    Grammar res = grammar;
    Production prod;
    prod.left = res.augmented_start_symbol;
    prod.right.push_back(res.start_symbol);
    res.productions.insert(res.productions.begin(), prod);
    res.nonterminals.insert(res.augmented_start_symbol);
    return res;
}

// 求项目集闭包Closure(I)
vector<Item> closure(const Grammar &grammar, const vector<Item> &seed)
{
    vector<Item> res = seed;
    for (int i = 0; i < (int)res.size(); i++)
    {
        Item item = res[i];
        Production prod = grammar.productions[item.production_index];
        if (item.dot_pos >= (int)prod.right.size()) continue;
        string symbol = prod.right[item.dot_pos];
        if (grammar.nonterminals.count(symbol) == 0) continue;
        for (int j = 0; j < (int)grammar.productions.size(); j++)
        {
            if (grammar.productions[j].left != symbol) continue;
            Item new_item;
            new_item.production_index = j;
            new_item.dot_pos = 0;
            bool existed = false;
            for (int k = 0; k < (int)res.size(); k++)
            {
                if (res[k].production_index == new_item.production_index &&
                    res[k].dot_pos == new_item.dot_pos)
                {
                    existed = true;
                    break;
                }
            }
            if (!existed) res.push_back(new_item);
        }
    }
    return res;
}

// 求项目集转移Goto(I, X)
vector<Item> go_to(const Grammar &grammar, const vector<Item> &items, const string &symbol)
{
    vector<Item> moved;
    for (int i = 0; i < (int)items.size(); i++)
    {
        Item item = items[i];
        Production prod = grammar.productions[item.production_index];
        if (item.dot_pos >= (int)prod.right.size())
            continue;
        if (prod.right[item.dot_pos] == symbol)
        {
            Item new_item;
            new_item.production_index = item.production_index;
            new_item.dot_pos = item.dot_pos + 1;
            moved.push_back(new_item);
        }
    }
    return closure(grammar, moved);
}

// 构造LR(0)项目集规范族
vector<ItemSet> build_canonical_collection(const Grammar &grammar)
{
    Item item;
    item.production_index = 0;
    item.dot_pos = 0;
    vector<Item> seed;
    seed.push_back(item);
    ItemSet item_set;
    item_set.id = 0;
    item_set.items = closure(grammar, seed);
    vector<ItemSet> item_sets;
    item_sets.push_back(item_set);

    vector<string> symbols;
    for (set<string>::iterator it = grammar.nonterminals.begin(); it != grammar.nonterminals.end(); ++it)
        symbols.push_back(*it);
    for (set<string>::iterator it = grammar.terminals.begin(); it != grammar.terminals.end(); ++it)
        symbols.push_back(*it);

    for (int i = 0; i < (int)item_sets.size(); i++)
    {
        for (int j = 0; j < (int)symbols.size(); j++)
        {
            vector<Item> next_items = go_to(grammar, item_sets[i].items, symbols[j]);
            if (next_items.empty()) continue;
            bool existed = false;
            for (int k = 0; k < (int)item_sets.size(); k++)
            {
                if ((int)item_sets[k].items.size() != (int)next_items.size()) continue;
                bool same = true;
                for (int a = 0; a < (int)next_items.size(); a++)
                {
                    bool found = false;
                    for (int b = 0; b < (int)item_sets[k].items.size(); b++)
                    {
                        if (next_items[a].production_index == item_sets[k].items[b].production_index &&
                            next_items[a].dot_pos == item_sets[k].items[b].dot_pos)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        same = false;
                        break;
                    }
                }
                if (same)
                {
                    existed = true;
                    break;
                }
            }
            if (!existed)
            {
                ItemSet new_set;
                new_set.id = (int)item_sets.size();
                new_set.items = next_items;
                item_sets.push_back(new_set);
            }
        }
    }
    return item_sets;
}

// 在已有项目集中查找与给定项目集相同的编号，找不到返回-1
int find_item_set(const vector<ItemSet> &item_sets, const vector<Item> &items)
{
    for (int k = 0; k < (int)item_sets.size(); k++)
    {
        if ((int)item_sets[k].items.size() != (int)items.size()) continue;
        bool same = true;
        for (int a = 0; a < (int)items.size(); a++)
        {
            bool found = false;
            for (int b = 0; b < (int)item_sets[k].items.size(); b++)
            {
                if (items[a].production_index == item_sets[k].items[b].production_index &&
                    items[a].dot_pos == item_sets[k].items[b].dot_pos)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                same = false;
                break;
            }
        }
        if (same) return item_sets[k].id;
    }
    return -1;
}

// 记录项目集之间的转移（Goto）。SLR表的移进项与GOTO项都来自这里。
vector<Transition> build_transitions(const Grammar &grammar, const vector<ItemSet> &item_sets)
{
    vector<string> symbols;
    for (set<string>::iterator it = grammar.nonterminals.begin(); it != grammar.nonterminals.end(); ++it)
        symbols.push_back(*it);
    for (set<string>::iterator it = grammar.terminals.begin(); it != grammar.terminals.end(); ++it)
        symbols.push_back(*it);

    vector<Transition> transitions;
    for (int i = 0; i < (int)item_sets.size(); i++)
    {
        for (int j = 0; j < (int)symbols.size(); j++)
        {
            vector<Item> next_items = go_to(grammar, item_sets[i].items, symbols[j]);
            if (next_items.empty()) continue;
            int to = find_item_set(item_sets, next_items);
            if (to < 0) continue;
            Transition transition;
            transition.from = item_sets[i].id;
            transition.symbol = symbols[j];
            transition.to = to;
            transitions.push_back(transition);
        }
    }
    return transitions;
}

// 检查LR(0)冲突（仅用于展示：说明该文法为什么不是LR(0)、从而需要SLR(1)）
vector<string> detect_lr0_conflicts(const Grammar &grammar, const vector<ItemSet> &item_sets)
{
    vector<string> conflicts;
    for (int i = 0; i < (int)item_sets.size(); i++)
    {
        int reduce_count = 0;
        bool has_shift = false;
        for (int j = 0; j < (int)item_sets[i].items.size(); j++)
        {
            Item item = item_sets[i].items[j];
            Production prod = grammar.productions[item.production_index];

            if (item.dot_pos >= (int)prod.right.size())
            {
                if (item.production_index != 0) reduce_count++;
            }
            else
            {
                string symbol = prod.right[item.dot_pos];
                if (grammar.terminals.count(symbol) > 0) has_shift = true;
            }
        }
        if (reduce_count > 0 && has_shift)
            conflicts.push_back("I" + to_string(item_sets[i].id) + " 存在移进-归约冲突");
        if (reduce_count > 1)
            conflicts.push_back("I" + to_string(item_sets[i].id) + " 存在归约-归约冲突");
    }
    return conflicts;
}

map<string, set<string>> compute_first(const Grammar &grammar)
{
    map<string, set<string>> first;

    // 规则1：终结符的 FIRST 就是它自身
    for (set<string>::iterator it = grammar.terminals.begin(); it != grammar.terminals.end(); ++it)
        first[*it].insert(*it);

    // 规则2+3+4：不动点迭代
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < (int)grammar.productions.size(); i++)
        {
            const string &A = grammar.productions[i].left;
            const vector<string> &rhs = grammar.productions[i].right;

            if (rhs.empty()) // 规则2：ε 产生式
            {
                if (first[A].insert("e").second) changed = true;
                continue;
            }

            // 规则3：逐符号扫描，直到遇到不含 ε 的符号
            bool all_nullable = true;
            for (int j = 0; j < (int)rhs.size(); j++)
            {
                const string &Yj = rhs[j];
                for (set<string>::iterator it = first[Yj].begin(); it != first[Yj].end(); ++it)
                    if (*it != "e" && first[A].insert(*it).second) changed = true;
                if (first[Yj].find("e") == first[Yj].end()) { all_nullable = false; break; }
            }
            if (all_nullable)
                if (first[A].insert("e").second) changed = true;
        }
    }

    return first;
}

map<string, set<string>> compute_follow(const Grammar &grammar,
                                        const map<string, set<string>> &first)
{
    map<string, set<string>> follow;

    // 规则1：原文法开始符号的 FOLLOW 集初始含 $
    follow[grammar.start_symbol].insert("$");

    // 规则2+3：不动点迭代
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < (int)grammar.productions.size(); i++)
        {
            const string &A = grammar.productions[i].left;
            const vector<string> &rhs = grammar.productions[i].right;
            for (int j = 0; j < (int)rhs.size(); j++)
            {
                const string &B = rhs[j];
                if (grammar.nonterminals.count(B) == 0) continue;

                // 从 B 后面逐符号扫描 β，直到遇到不含 ε 的符号
                bool nullable = true;
                for (int k = j + 1; k < (int)rhs.size(); k++)
                {
                    const string &sym = rhs[k];
                    const set<string> &fs = first.at(sym);
                    for (set<string>::iterator it = fs.begin(); it != fs.end(); ++it)
                        if (*it != "e" && follow[B].insert(*it).second) changed = true;
                    if (fs.find("e") == fs.end()) { nullable = false; break; }
                }
                // β 整体可推 ε（或 B 在末尾）：把 FOLLOW(A) 加入 FOLLOW(B)
                if (nullable)
                    for (set<string>::iterator it = follow[A].begin(); it != follow[A].end(); ++it)
                        if (follow[B].insert(*it).second) changed = true;
            }
        }
    }

    return follow;
}

vector<map<string, Action>> build_action_table(const Grammar &grammar,
                                               const vector<ItemSet> &item_sets,
                                               const vector<Transition> &transitions,
                                               const map<string, set<string>> &follow,
                                               vector<string> &conflicts)
{
    int n = (int)item_sets.size();
    vector<map<string, Action>> action(n);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < (int)item_sets[i].items.size(); j++)
        {
            const Item &item = item_sets[i].items[j];
            const Production &prod = grammar.productions[item.production_index];

            if (item.dot_pos < (int)prod.right.size())
            {
                // 情况A：移进项目，点后是终结符 a
                const string &a = prod.right[item.dot_pos];
                if (grammar.terminals.count(a) == 0) continue;

                // 在 transitions 里找 Ii --a--> Ij
                for (int k = 0; k < (int)transitions.size(); k++)
                {
                    if (transitions[k].from != i || transitions[k].symbol != a) continue;
                    Action shift;
                    shift.type = 1;
                    shift.value = transitions[k].to;

                    if (action[i].count(a) && action[i][a].type == 2)
                    {
                        // 移进-归约冲突：采用"移进优先"（解决悬挂 else 等），覆盖原归约
                        conflicts.push_back("I" + to_string(i) + " 在符号 " + a + " 上存在移进-归约冲突（移进优先）");
                        action[i][a] = shift;
                    }
                    else if (action[i].count(a) == 0)
                        action[i][a] = shift;
                    break;
                }
            }
            else
            {
                // 点在最右端
                if (item.production_index == 0)
                {
                    // 情况C：接受项目 S' -> S·
                    Action acc;
                    acc.type = 3;
                    acc.value = 0;
                    if (action[i].count("$") == 0)
                        action[i]["$"] = acc;
                }
                else
                {
                    // 情况B：归约项目，对 FOLLOW(A) 中每个符号填 rk
                    const string &A = prod.left;
                    if (follow.count(A) == 0) continue;
                    const set<string> &fa = follow.at(A);
                    for (set<string>::iterator it = fa.begin(); it != fa.end(); ++it)
                    {
                        const string &a = *it;
                        Action reduce;
                        reduce.type = 2;
                        reduce.value = item.production_index;

                        if (action[i].count(a))
                        {
                            if (action[i][a].type == 1)
                                conflicts.push_back("I" + to_string(i) + " 在符号 " + a + " 上存在移进-归约冲突");
                            else if (action[i][a].type == 2 && action[i][a].value != reduce.value)
                                conflicts.push_back("I" + to_string(i) + " 在符号 " + a + " 上存在归约-归约冲突");
                        }
                        else
                        {
                            action[i][a] = reduce;
                        }
                    }
                }
            }
        }
    }

    return action;
}

vector<map<string, int>> build_goto_table(const Grammar &grammar,
                                          const vector<ItemSet> &item_sets,
                                          const vector<Transition> &transitions)
{
    int n = (int)item_sets.size();
    vector<map<string, int>> go_to(n);

    for (int i = 0; i < (int)transitions.size(); i++)
    {
        const string &X = transitions[i].symbol;
        if (grammar.nonterminals.count(X) == 0) continue; // 只处理非终结符
        go_to[transitions[i].from][X] = transitions[i].to;
    }

    return go_to;
}

// 组装 SLR(1) 分析表
SLR1Table build_slr1_table(const Grammar &grammar, const vector<ItemSet> &item_sets,
                           const vector<Transition> &transitions,
                           const map<string, set<string>> &follow)
{
    SLR1Table table;
    // ACTION 表列头：所有终结符（set 已按字典序），末尾再加输入结束符 $
    for (set<string>::iterator it = grammar.terminals.begin(); it != grammar.terminals.end(); ++it)
        table.terminals.push_back(*it);
    table.terminals.push_back("$");
    // GOTO 表列头：所有非终结符，但去掉增广开始符号 S'
    for (set<string>::iterator it = grammar.nonterminals.begin(); it != grammar.nonterminals.end(); ++it)
        if (*it != grammar.augmented_start_symbol)
            table.nonterminals.push_back(*it);
    // 调用待实现的填表函数
    table.action = build_action_table(grammar, item_sets, transitions, follow, table.conflicts);
    table.go_to = build_goto_table(grammar, item_sets, transitions);
    return table;
}

SLR1Result build_slr1(const vector<string> &lines)
{
    SLR1Result result;

    result.grammar = augment_grammar(parse_grammar(lines));
    result.item_sets = build_canonical_collection(result.grammar);
    result.transitions = build_transitions(result.grammar, result.item_sets);
    result.lr0_conflicts = detect_lr0_conflicts(result.grammar, result.item_sets);
    result.first = compute_first(result.grammar);                  // TODO 1
    result.follow = compute_follow(result.grammar, result.first);  // TODO 2
    result.table = build_slr1_table(result.grammar, result.item_sets,
                                    result.transitions, result.follow); // 内部调用 TODO 3 / TODO 4

    return result;
}

// ════════════════════════════════════════════════════════════════
//  实验五接口：导出 SLR(1) 分析表到文件（供 Lab5 读取）
// ════════════════════════════════════════════════════════════════
// 纯文本分节格式：
//   STATES <状态数>
//   PRODUCTIONS <产生式数>
//   <编号> <左部> <右部...>        // ε 产生式无右部
//   TERMINALS <终结符... $>
//   NONTERMINALS <非终结符...>
//   ACTION <表项数>
//   <状态> <终结符> <动作>          // 动作：sN 移进 / rN 归约 / acc 接受
//   GOTO <表项数>
//   <状态> <非终结符> <目标状态>
void export_slr1_table(const SLR1Result &result, const string &path)
{
    ofstream fout(path);
    if (!fout.is_open())
    {
        cerr << "无法写出分析表文件: " << path << endl;
        return;
    }

    const vector<Production> &prods = result.grammar.productions;
    const SLR1Table &t = result.table;

    fout << "STATES " << t.action.size() << "\n";

    fout << "PRODUCTIONS " << prods.size() << "\n";
    for (int i = 0; i < (int)prods.size(); i++)
    {
        fout << i << " " << prods[i].left;
        for (int j = 0; j < (int)prods[i].right.size(); j++)
            fout << " " << prods[i].right[j];
        fout << "\n";
    }

    fout << "TERMINALS";
    for (int i = 0; i < (int)t.terminals.size(); i++)
        fout << " " << t.terminals[i];
    fout << "\n";

    fout << "NONTERMINALS";
    for (int i = 0; i < (int)t.nonterminals.size(); i++)
        fout << " " << t.nonterminals[i];
    fout << "\n";

    int acount = 0;
    for (int s = 0; s < (int)t.action.size(); s++)
        acount += (int)t.action[s].size();
    fout << "ACTION " << acount << "\n";
    for (int s = 0; s < (int)t.action.size(); s++)
        for (map<string, Action>::const_iterator it = t.action[s].begin();
             it != t.action[s].end(); ++it)
        {
            const Action &a = it->second;
            string act;
            if (a.type == 1)      act = "s" + to_string(a.value);
            else if (a.type == 2) act = "r" + to_string(a.value);
            else if (a.type == 3) act = "acc";
            else continue;
            fout << s << " " << it->first << " " << act << "\n";
        }

    int gcount = 0;
    for (int s = 0; s < (int)t.go_to.size(); s++)
        gcount += (int)t.go_to[s].size();
    fout << "GOTO " << gcount << "\n";
    for (int s = 0; s < (int)t.go_to.size(); s++)
        for (map<string, int>::const_iterator it = t.go_to[s].begin();
             it != t.go_to[s].end(); ++it)
            fout << s << " " << it->first << " " << it->second << "\n";
}

//  输出函数

void print_grammar_lines(const vector<string> &lines)
{
    cout << "原始文法" << endl;
    for (int i = 0; i < (int)lines.size(); i++)
        cout << "  " << lines[i] << endl;
    cout << endl;
}

void print_symbol_set(const string &name, const set<string> &symbols)
{
    cout << name << ": ";
    for (set<string>::iterator it = symbols.begin(); it != symbols.end(); ++it)
        cout << *it << " ";
    cout << endl;
}

void print_parsed_grammar(const Grammar &grammar)
{
    cout << "文法解析结果" << endl;
    cout << "  开始符号: " << grammar.start_symbol << endl;
    cout << "  增广开始符号: " << grammar.augmented_start_symbol << endl;

    print_symbol_set("  非终结符", grammar.nonterminals);
    print_symbol_set("  终结符", grammar.terminals);

    cout << "  产生式:" << endl;
    for (int i = 0; i < (int)grammar.productions.size(); i++)
    {
        cout << "    " << i << ": " << grammar.productions[i].left << " -> ";
        for (int j = 0; j < (int)grammar.productions[i].right.size(); j++)
            cout << grammar.productions[i].right[j] << " ";
        cout << endl;
    }
    cout << endl;
}

void print_item(const Grammar &grammar, const Item &item)
{
    Production prod = grammar.productions[item.production_index];

    cout << "    " << prod.left << " -> ";
    for (int i = 0; i <= (int)prod.right.size(); i++)
    {
        if (i == item.dot_pos)
            cout << ". ";
        if (i < (int)prod.right.size())
            cout << prod.right[i] << " ";
    }
    cout << endl;
}

void print_item_sets(const Grammar &grammar, const vector<ItemSet> &item_sets)
{
    cout << "LR(0)项目集" << endl;
    for (int i = 0; i < (int)item_sets.size(); i++)
    {
        cout << "  I" << item_sets[i].id << ":" << endl;
        for (int j = 0; j < (int)item_sets[i].items.size(); j++)
            print_item(grammar, item_sets[i].items[j]);
        cout << endl;
    }
}

void print_transitions(const vector<Transition> &transitions)
{
    cout << "Goto转移" << endl;
    if (transitions.empty())
    {
        cout << "  无转移" << endl;
    }
    else
    {
        for (int i = 0; i < (int)transitions.size(); i++)
            cout << "  I" << transitions[i].from << " --" << transitions[i].symbol
                 << "--> I" << transitions[i].to << endl;
    }
    cout << endl;
}

void print_lr0_conflicts(const vector<string> &conflicts)
{
    cout << "LR(0)冲突检查（引入SLR(1)的动机）" << endl;
    if (conflicts.empty())
    {
        cout << "  未发现LR(0)冲突，该文法本身就是LR(0)文法" << endl;
    }
    else
    {
        cout << "  发现LR(0)冲突，需用FOLLOW集尝试消解：" << endl;
        for (int i = 0; i < (int)conflicts.size(); i++)
            cout << "  " << conflicts[i] << endl;
    }
    cout << endl;
}

// 打印 FIRST / FOLLOW 集（数据为空时提示对应 TODO 尚未实现）
void print_first_follow(const map<string, set<string>> &first,
                        const map<string, set<string>> &follow)
{
    cout << "FIRST 集" << endl;
    if (first.empty())
    {
        cout << "  （尚未实现，见 compute_first 的 TODO 1）" << endl;
    }
    else
    {
        for (map<string, set<string>>::const_iterator it = first.begin(); it != first.end(); ++it)
            print_symbol_set("  FIRST(" + it->first + ")", it->second);
    }
    cout << endl;

    cout << "FOLLOW 集" << endl;
    if (follow.empty())
    {
        cout << "  （尚未实现，见 compute_follow 的 TODO 2）" << endl;
    }
    else
    {
        for (map<string, set<string>>::const_iterator it = follow.begin(); it != follow.end(); ++it)
            print_symbol_set("  FOLLOW(" + it->first + ")", it->second);
    }
    cout << endl;
}

// 把一个 ACTION 动作转成显示字符串：移进 sN、归约 rN、接受 acc、空表项为空白
string action_to_string(const Action &action)
{
    if (action.type == 1) return "s" + to_string(action.value);
    if (action.type == 2) return "r" + to_string(action.value);
    if (action.type == 3) return "acc";
    return "";
}

// 打印 SLR(1) 分析表（ACTION + GOTO），并报告仍存在的冲突
void print_slr1_table(const SLR1Table &table)
{
    const int CW = 6;  // 每个数据列的宽度

    cout << "SLR(1) ACTION 表" << endl;
    if (table.action.empty())
    {
        cout << "  （尚未实现，见 build_action_table 的 TODO 3）" << endl;
    }
    else
    {
        cout << "  " << pad("状态", CW);
        for (int j = 0; j < (int)table.terminals.size(); j++)
            cout << pad(table.terminals[j], CW);
        cout << endl;
        for (int i = 0; i < (int)table.action.size(); i++)
        {
            cout << "  " << pad(to_string(i), CW);
            for (int j = 0; j < (int)table.terminals.size(); j++)
            {
                string cell = "";
                map<string, Action>::const_iterator it = table.action[i].find(table.terminals[j]);
                if (it != table.action[i].end())
                    cell = action_to_string(it->second);
                cout << pad(cell, CW);
            }
            cout << endl;
        }
    }
    cout << endl;

    cout << "SLR(1) GOTO 表" << endl;
    if (table.go_to.empty())
    {
        cout << "  （尚未实现，见 build_goto_table 的 TODO 4）" << endl;
    }
    else
    {
        cout << "  " << pad("状态", CW);
        for (int j = 0; j < (int)table.nonterminals.size(); j++)
            cout << pad(table.nonterminals[j], CW);
        cout << endl;
        for (int i = 0; i < (int)table.go_to.size(); i++)
        {
            cout << "  " << pad(to_string(i), CW);
            for (int j = 0; j < (int)table.nonterminals.size(); j++)
            {
                string cell = "";
                map<string, int>::const_iterator it = table.go_to[i].find(table.nonterminals[j]);
                if (it != table.go_to[i].end())
                    cell = to_string(it->second);
                cout << pad(cell, CW);
            }
            cout << endl;
        }
    }
    cout << endl;

    cout << "SLR(1) 冲突检查" << endl;
    if (table.action.empty() && table.go_to.empty())
    {
        cout << "  （分析表尚未实现，无法判定）" << endl;
    }
    else if (table.conflicts.empty())
    {
        cout << "  未发现冲突，该文法是 SLR(1) 文法" << endl;
    }
    else
    {
        cout << "  发现冲突，该文法不是 SLR(1) 文法：" << endl;
        for (int i = 0; i < (int)table.conflicts.size(); i++)
            cout << "  " << table.conflicts[i] << endl;
    }
    cout << endl;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    string grammar_path = "grammar.txt";
    string table_out = "slr1_table.txt";  // 供 Lab5 读取的分析表文件
    if (argc >= 2)
        grammar_path = argv[1];
    if (argc >= 3)
        table_out = argv[2];

    vector<string> lines = read_grammar_lines(grammar_path);
    SLR1Result result = build_slr1(lines);

    cout << "SLR(1)分析表生成程序" << endl;
    cout << "文法文件: " << grammar_path << endl;
    cout << endl;

    print_grammar_lines(lines);
    print_parsed_grammar(result.grammar);

    cout << "增广文法" << endl;
    cout << "  " << result.grammar.productions[0].left << " -> ";
    for (int i = 0; i < (int)result.grammar.productions[0].right.size(); i++)
        cout << result.grammar.productions[0].right[i] << " ";
    cout << endl;
    cout << endl;

    print_item_sets(result.grammar, result.item_sets);
    print_transitions(result.transitions);
    print_lr0_conflicts(result.lr0_conflicts);
    print_first_follow(result.first, result.follow);
    print_slr1_table(result.table);

    // 实验五接口：把分析表导出到文件
    export_slr1_table(result, table_out);
    cout << "SLR(1) 分析表已导出到: " << table_out << endl;

    return 0;
}