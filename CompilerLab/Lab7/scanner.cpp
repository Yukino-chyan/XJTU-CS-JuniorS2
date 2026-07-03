#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ast.h"   // TokenRecord（供 Lab6 进程内取 token 流）

// Token 类型
typedef enum
{
    // 标识符、关键字
    TOKEN_ID,
    TOKEN_INT, TOKEN_FLOAT, TOKEN_VOID,
    TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_RETURN, TOKEN_INPUT, TOKEN_PRINT,
    // 数字常量
    TOKEN_NUM, TOKEN_FLO,
    // 算术运算符
    TOKEN_ADD, TOKEN_SUB, TOKEN_MUL, TOKEN_DIV,
    // 赋值运算符
    TOKEN_ASG, TOKEN_AAS, TOKEN_AAA,
    // 关系运算符
    TOKEN_LT, TOKEN_LE, TOKEN_EQ, TOKEN_GT, TOKEN_GE, TOKEN_NE,
    // 逻辑运算符
    TOKEN_AND, TOKEN_OR, TOKEN_NOT,
    // 括号与界符
    TOKEN_LPA, TOKEN_RPA,
    TOKEN_LBK, TOKEN_RBK,
    TOKEN_LBR, TOKEN_RBR,
    TOKEN_CMA, TOKEN_SCO,
    // 错误
    TOKEN_ERROR
} TokenType;

// DFA 状态
typedef enum
{
    S_START,    // 初始状态
    S_ID,       // 正在读标识符/关键字
    S_NUM,      // 正在读整数
    S_FLO_DOT,      // 读到小数点（从 S_NUM 来，如 "123."，可直接终止）
    S_FLO_DOT_START,// 读到小数点（从 S_START 来，如 ".45"，后面必须有数字）
    S_FLO_DEC,  // 读到小数点后的数字
    S_FLO_E,    // 读到 e/E
    S_FLO_SIGN, // 读到 e/E 后的正负号
    S_FLO_EXP,  // 读到指数部分的数字
    S_ADD,      // 读到 +
    S_SUB,      // 读到 -
    S_MUL,      // 读到 *（直接终态）
    S_DIV,      // 读到 /（直接终态）
    S_ASG,      // 读到 =
    S_LT,       // 读到 <
    S_GT,       // 读到 >
    S_NOT,      // 读到 !
    S_AND,      // 读到 &
    S_OR,       // 读到 |
} DFAState;

// Token 结构体
struct Token
{
    TokenType type;
    char original[256]; // 原始字符串
};

// 关键字表：区分标识符和关键字
struct Keyword
{
    const char *word;
    TokenType   type;
};

static Keyword keywords[] =
{
    {"int", TOKEN_INT},
    {"float", TOKEN_FLOAT},
    {"void", TOKEN_VOID},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN},
    {"input", TOKEN_INPUT   },
    {"print", TOKEN_PRINT   },
};
static int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

// 在关键字表中查找，找到返回对应类型，否则返回 TOKEN_ID
TokenType lookup_keyword(const char *s)
{
    for(int i=0; i<keyword_count; i++)
    {
        if (strcmp(s, keywords[i].word) == 0)
            return keywords[i].type;
    }
    return TOKEN_ID;
}

// Token 转字符串
const char *token_to_str(TokenType t)
{
    switch (t)
    {
        case TOKEN_ID:     return "ID";
        case TOKEN_INT:    return "INT";
        case TOKEN_FLOAT:  return "FLOAT";
        case TOKEN_VOID:   return "VOID";
        case TOKEN_IF:     return "IF";
        case TOKEN_ELSE:   return "ELSE";
        case TOKEN_WHILE:  return "WHILE";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_INPUT:  return "INPUT";
        case TOKEN_PRINT:  return "PRINT";
        case TOKEN_NUM:    return "NUM";
        case TOKEN_FLO:    return "FLO";
        case TOKEN_ADD:    return "ADD";
        case TOKEN_SUB:    return "SUB";
        case TOKEN_MUL:    return "MUL";
        case TOKEN_DIV:    return "DIV";
        case TOKEN_ASG:    return "ASG";
        case TOKEN_AAS:    return "AAS";
        case TOKEN_AAA:    return "AAA";
        case TOKEN_LT:     return "LT";
        case TOKEN_LE:     return "LE";
        case TOKEN_EQ:     return "EQ";
        case TOKEN_GT:     return "GT";
        case TOKEN_GE:     return "GE";
        case TOKEN_NE:     return "NE";
        case TOKEN_AND:    return "AND";
        case TOKEN_OR:     return "OR";
        case TOKEN_NOT:    return "NOT";
        case TOKEN_LPA:    return "LPA";
        case TOKEN_RPA:    return "RPA";
        case TOKEN_LBK:    return "LBK";
        case TOKEN_RBK:    return "RBK";
        case TOKEN_LBR:    return "LBR";
        case TOKEN_RBR:    return "RBR";
        case TOKEN_CMA:    return "CMA";
        case TOKEN_SCO:    return "SCO";
        default:           return "ERROR";
    }
}

// DFA 扫描函数
// 从字符串 src 的位置 *pos 开始，识别一个 Token
// 识别完后 *pos 指向下一个未处理字符
Token next_token(const char *src, int *pos)
{
    Token tok;
    tok.type = TOKEN_ERROR;
    tok.original[0] = '\0';

    // 跳过空白符
    while (src[*pos] != '\0' && isspace((unsigned char)src[*pos])) (*pos)++;
    if (src[*pos] == '\0')
    {
        tok.type = TOKEN_ERROR;
        return tok;
    }
    DFAState state = S_START;
    int len = 0;
    char buf[256];
    while (1)
    {
        char c = src[*pos];
        switch (state)
        {
            case S_START:
                buf[len++] = c;
                (*pos)++;
                if      (isalpha((unsigned char)c)) state = S_ID;
                else if (isdigit((unsigned char)c)) state = S_NUM;
                else if (c == '+') state = S_ADD;
                else if (c == '-') state = S_SUB;
                else if (c == '.') state = S_FLO_DOT_START;
                else if (c == '=') state = S_ASG;
                else if (c == '<') state = S_LT;
                else if (c == '>') state = S_GT;
                else if (c == '!') state = S_NOT;
                else if (c == '&') state = S_AND;
                else if (c == '|') state = S_OR;
                else if (c == '*') tok.type = TOKEN_MUL;
                else if (c == '/') tok.type = TOKEN_DIV;
                else if (c == '(') tok.type = TOKEN_LPA;
                else if (c == ')') tok.type = TOKEN_RPA;
                else if (c == '[') tok.type = TOKEN_LBK;
                else if (c == ']') tok.type = TOKEN_RBK;
                else if (c == '{') tok.type = TOKEN_LBR;
                else if (c == '}') tok.type = TOKEN_RBR;
                else if (c == ',') tok.type = TOKEN_CMA;
                else if (c == ';') tok.type = TOKEN_SCO;
                else               tok.type = TOKEN_ERROR;
                break;

            case S_ID:
                if (isalpha((unsigned char)c) || isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = lookup_keyword(buf);
                }
                break;

            case S_NUM:
                if (isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                }
                else if (c == '.')
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_DOT;
                }
                else if (c == 'e' || c == 'E')
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_E;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_NUM;
                }
                break;

            case S_FLO_DOT:
                if (isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_DEC;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_FLO;
                }
                break;

            case S_FLO_DOT_START:
                if (isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_DEC;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_ERROR;
                }
                break;

            case S_FLO_DEC:
                if (isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                }
                else if (c == 'e' || c == 'E')
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_E;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_FLO;
                }
                break;

            case S_FLO_E:
                if (c == '+' || c == '-')
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_SIGN;
                }
                else if (isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_EXP;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_ERROR;
                }
                break;

            case S_FLO_SIGN:
                if (isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                    state = S_FLO_EXP;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_ERROR;
                }
                break;

            case S_FLO_EXP:
                if (isdigit((unsigned char)c))
                {
                    buf[len++] = c;
                    (*pos)++;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_FLO;
                }
                break;

            case S_ADD:
                if (c == '=')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_AAS;
                }
                else if (c == '+')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_AAA;
                }
                else if (isdigit((unsigned char)c))
                {
                    state = S_NUM;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_ADD;
                }
                break;

            case S_SUB:
                if (isdigit((unsigned char)c))
                {
                    state = S_NUM;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_SUB;
                }
                break;

            case S_ASG:
                if (c == '=')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_EQ;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_ASG;
                }
                break;

            case S_LT:
                if (c == '=')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_LE;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_LT;
                }
                break;

            case S_GT:
                if (c == '=')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_GE;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_GT;
                }
                break;

            case S_NOT:
                if (c == '=')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_NE;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_NOT;
                }
                break;

            case S_AND:
                if (c == '&')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_AND;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_ERROR;
                }
                break;

            case S_OR:
                if (c == '|')
                {
                    buf[len++] = c;
                    (*pos)++;
                    tok.type = TOKEN_OR;
                }
                else
                {
                    buf[len] = '\0';
                    tok.type = TOKEN_ERROR;
                }
                break;

            default:
                break;
        }
        //已识别完一个 Token
        if (tok.type != TOKEN_ERROR || state == S_START) break;
    }

    buf[len] = '\0';
    if (strlen(tok.original) == 0) strcpy(tok.original, buf);

    return tok;
}

// 对一行字符串做词法分析，逐个打印 Token
void scan_line(const char *src)
{
    int pos = 0;
    while (src[pos] != '\0')
    {
        while (src[pos] != '\0' && isspace((unsigned char)src[pos])) pos++;
        if (src[pos] == '\0') break;
        Token tok = next_token(src, &pos);
        printf("(%s, %s)\n", token_to_str(tok.type), tok.original);
    }
}

// 输入 n 个空格分隔的符号串，逐个判断类型
void mode1()
{
    int n;
    scanf("%d", &n);
    for(int i=1; i<=n; i++)
    {
        char buf[256];
        scanf("%s", buf);
        int pos = 0;
        Token tok = next_token(buf, &pos);
        printf("(%s, %s)\n", token_to_str(tok.type), tok.original);
    }
}

// 输入一整行语句，输出 Token 流
void mode2()
{
    char line[1024];
    getchar();
    fgets(line, sizeof(line), stdin);
    scan_line(line);
}

// Lab6 接口：对整段源码做词法分析，返回 token 序列
// 规则与 main/scan_line 一致：按行处理、去掉 // 行注释，逐个识别 token
vector<TokenRecord> tokenize(const string& src)
{
    vector<TokenRecord> out;
    size_t start = 0;
    while (start <= src.size())
    {
        size_t nl = src.find('\n', start);
        string line = (nl == string::npos) ? src.substr(start) : src.substr(start, nl - start);
        start = (nl == string::npos) ? src.size() + 1 : nl + 1;

        size_t cmt = line.find("//");          // 去掉 // 行注释（含中文标注）
        if (cmt != string::npos) line = line.substr(0, cmt);

        const char* s = line.c_str();
        int pos = 0;
        while (s[pos] != '\0')
        {
            while (s[pos] != '\0' && isspace((unsigned char)s[pos])) pos++;
            if (s[pos] == '\0') break;
            Token tok = next_token(s, &pos);
            out.push_back(TokenRecord{ token_to_str(tok.type), tok.original });
        }
    }
    return out;
}

#ifndef LAB6_BUILD   // 独立构建 scanner.exe 时保留 main；被 Lab6 链接时（-DLAB6_BUILD）排除
int main(int argc, char *argv[])
{
    const char *in_path  = (argc >= 2) ? argv[1] : "test.c";
    const char *out_path = (argc >= 3) ? argv[2] : "out.txt";
    freopen(in_path,  "r", stdin);
    freopen(out_path, "w", stdout);

    char line[1024];
    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        // 去掉 // 行注释（含测试用例里的中文标注），避免被误当作 token
        char *cmt = strstr(line, "//");
        if (cmt != NULL)
        {
            cmt[0] = '\n';
            cmt[1] = '\0';
        }
        scan_line(line);
    }

    fclose(stdin);
    fclose(stdout);
    return 0;
}
#endif // LAB6_BUILD