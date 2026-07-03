#pragma once
// SLR(1) 分析表的内存表示（读实验四导出的表用）+ 字符串工具。

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// 数据结构（与 Lab4 一致）
struct Production { string left; vector<string> right; }; // right 为空 = ε 产生式
struct Action     { int type; int value; };  // 0=err 1=shift 2=reduce 3=accept

struct SLR1Table
{
    vector<string> terminals;             // ACTION 列头（含 $）
    vector<string> nonterminals;          // GOTO 列头
    vector<map<string, Action>> action;   // action[状态][终结符]
    vector<map<string, int>>    go_to;    // go_to[状态][非终结符]
};

// 读入结果：产生式表 + 分析表
struct SLR1Result
{
    vector<Production> productions;
    SLR1Table table;
};

// 字符串工具
inline string trim(const string& s)
{
    const string b = " \t\r\n";
    auto f = s.find_first_not_of(b), l = s.find_last_not_of(b);
    return f == string::npos ? "" : s.substr(f, l - f + 1);
}

inline vector<string> split_symbols(const string& s)
{
    vector<string> res; string t; istringstream in(s);
    while (in >> t) res.push_back(t);
    return res;
}

inline int display_width(const string& s)
{
    int w = 0;
    for (int i = 0; i < (int)s.size(); )
    {
        unsigned char c = s[i];
        if      (c < 0x80) { w += 1; i += 1; }
        else if (c < 0xE0) { w += 1; i += 2; }
        else if (c < 0xF0) { w += 2; i += 3; }
        else               { w += 2; i += 4; }
    }
    return w;
}

inline string pad(const string& s, int width)
{
    string res = s;
    for (int i = display_width(s); i < width; i++) res += ' ';
    return res;
}
