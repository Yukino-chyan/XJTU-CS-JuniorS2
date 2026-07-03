
#include <fstream>
#include <iostream>
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

// LR(0)构造结果
struct LR0Result
{
    Grammar grammar;
    vector<ItemSet> item_sets;
    vector<Transition> transitions;
    vector<string> conflicts;
};

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
        if (!line.empty())
            lines.push_back(line);
    }

    return lines;
}

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
            prod.right = split_symbols(candidate);
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

// 增广文法
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

// 记录项目集之间的转移
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

// 检查LR(0)冲突
vector<string> detect_conflicts(const Grammar &grammar, const vector<ItemSet> &item_sets)
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

LR0Result build_lr0(const vector<string> &lines)
{
    LR0Result result;

    result.grammar = augment_grammar(parse_grammar(lines));
    result.item_sets = build_canonical_collection(result.grammar);
    result.transitions = build_transitions(result.grammar, result.item_sets);
    result.conflicts = detect_conflicts(result.grammar, result.item_sets);

    return result;
}

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

void print_result_skeleton(const LR0Result &result)
{
    print_parsed_grammar(result.grammar);

    cout << "增广文法" << endl;
    cout << "  " << result.grammar.productions[0].left << " -> ";
    for (int i = 0; i < (int)result.grammar.productions[0].right.size(); i++)
        cout << result.grammar.productions[0].right[i] << " ";
    cout << endl;
    cout << endl;

    print_item_sets(result.grammar, result.item_sets);

    cout << "Goto转移" << endl;
    if (result.transitions.empty())
    {
        cout << "  无转移" << endl;
    }
    else
    {
        for (int i = 0; i < (int)result.transitions.size(); i++)
            cout << "  I" << result.transitions[i].from << " --" << result.transitions[i].symbol
                 << "--> I" << result.transitions[i].to << endl;
    }
    cout << endl;

    cout << "冲突检查" << endl;
    if (result.conflicts.empty())
    {
        cout << "  未发现LR(0)冲突，该文法可认为是LR(0)文法" << endl;
    }
    else
    {
        cout << "  发现LR(0)冲突：" << endl;
        for (int i = 0; i < (int)result.conflicts.size(); i++)
            cout << "  " << result.conflicts[i] << endl;
    }
    cout << endl;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    string grammar_path = "grammar.txt";
    if (argc >= 2)
        grammar_path = argv[1];

    vector<string> lines = read_grammar_lines(grammar_path);
    LR0Result result = build_lr0(lines);

    cout << "LR(0)项目集规范族构造程序" << endl;
    cout << "文法文件: " << grammar_path << endl;
    cout << endl;

    print_grammar_lines(lines);
    print_result_skeleton(result);

    return 0;
}