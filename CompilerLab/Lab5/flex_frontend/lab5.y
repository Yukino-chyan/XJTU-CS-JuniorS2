%{
#include <cstdio>
#include <cstring>
#include "frontend.h"      // ASTNode / make_node / add_child / splice / make_binop / analyze / print_*
#ifdef _WIN32
// 不直接 include <windows.h>：它把 INT/FLOAT/VOID 定义成类型别名，会和 token 名冲突。
// 只前向声明需要的这一个函数（链接时由 kernel32 提供）。
extern "C" int __stdcall SetConsoleOutputCP(unsigned int);
#endif

extern int  yylex(void);
extern FILE* yyin;
void yyerror(const char* s) { fprintf(stderr, "语法错误: %s\n", s); }

ASTNode* g_root = nullptr;   // 解析得到的 AST 根
%}

/* 前向声明放进生成的 lab5.tab.h，使 union 里的 ASTNode* 在 lex.yy.c 中也可用 */
%code requires {
    struct ASTNode;
}

%union {
    char*    str;     // ID / NUM / FLO 的词素
    ASTNode* node;    // AST 子树
}

%token <str> ID NUM FLO
%token INT FLOAT VOID IF ELSE WHILE RETURN PRINT
%token ADD SUB MUL DIV
%token LT LE EQ GT GE NE
%token ASG LPA RPA LBK RBK LBR RBR CMA SCO

%type <node> DeclList Decl Type ParamList Param
%type <node> StmtList Stmt Bool Expr Term Fact ArgList Arg

%expect 1   /* 悬挂 else：1 个移进-归约冲突，bison 默认移进（else 配最近 if），符合预期 */

%%

/* 程序 = 声明表 + 入口语句表 */
Prog : DeclList StmtList
         { g_root = make_node(AST_PROGRAM); splice(g_root, $1); splice(g_root, $2); }
     ;

/* 声明表：分号分隔，可空 */
DeclList : DeclList Decl SCO  { $$ = $1; add_child($$, $2); }
         | /* 空 */           { $$ = make_node(AST_DECL_LIST); }
         ;

/* 声明：变量 / 带初值变量 / 数组 / 函数 */
Decl : Type ID
         { $$ = make_node(AST_VAR_DECL, $2, $1->val_type.c_str()); }
     | Type ID ASG Expr
         { $$ = make_node(AST_VAR_DECL, $2, $1->val_type.c_str()); add_child($$, $4); }
     | Type ID LBK NUM RBK
         { $$ = make_node(AST_VAR_DECL, $2, ($1->val_type + "[]").c_str()); }
     | Type ID LPA ParamList RPA LBR DeclList StmtList RBR
         { $$ = make_node(AST_FUN_DECL, $2, $1->val_type.c_str());
           splice($$, $4); splice($$, $7); splice($$, $8); }
     ;

Type : INT    { $$ = make_node(AST_TYPE, "int",   "int");   }
     | FLOAT  { $$ = make_node(AST_TYPE, "float", "float"); }
     | VOID   { $$ = make_node(AST_TYPE, "void",  "void");  }
     ;

/* 形参表：分号分隔，可空 */
ParamList : ParamList Param SCO { $$ = $1; add_child($$, $2); }
          | /* 空 */            { $$ = make_node(AST_PARAM_LIST); }
          ;

Param : Type ID          { $$ = make_node(AST_PARAM, $2, $1->val_type.c_str()); }
      | Type ID LBK RBK  { $$ = make_node(AST_PARAM, $2, ($1->val_type + "[]").c_str()); }
      ;

/* 语句表：分号分隔，允许尾随分号 */
StmtList : StmtList SCO Stmt { $$ = $1; add_child($$, $3); }
         | Stmt              { $$ = make_node(AST_STMT_LIST); add_child($$, $1); }
         | StmtList SCO      { $$ = $1; }
         ;

/* 语句 */
Stmt : ID ASG Expr
         { ASTNode* t = make_node(AST_ID, $1);
           $$ = make_node(AST_ASSIGN); add_child($$, t); add_child($$, $3); }
     | ID LBK Expr RBK ASG Expr
         { ASTNode* t = make_node(AST_ARRAY_ACCESS, $1); add_child(t, $3);
           $$ = make_node(AST_ASSIGN); add_child($$, t); add_child($$, $6); }
     | IF LPA Bool RPA Stmt
         { $$ = make_node(AST_IF_STMT); add_child($$, $3); add_child($$, $5); }
     | IF LPA Bool RPA Stmt ELSE Stmt
         { $$ = make_node(AST_IF_STMT); add_child($$, $3); add_child($$, $5); add_child($$, $7); }
     | WHILE LPA Bool RPA Stmt
         { $$ = make_node(AST_WHILE_STMT); add_child($$, $3); add_child($$, $5); }
     | RETURN Expr
         { $$ = make_node(AST_RETURN_STMT); add_child($$, $2); }
     | LBR StmtList RBR
         { $$ = make_node(AST_COMP_STMT); splice($$, $2); }
     | ID LPA ArgList RPA
         { $$ = make_node(AST_CALL, $1); splice($$, $3); }
     | PRINT LPA Expr RPA
         { $$ = make_node(AST_PRINT_STMT); add_child($$, $3); }
     ;

/* 布尔表达式（关系运算直接内联，避免单独的 RelOp 传字符串） */
Bool : Expr LT Expr { $$ = make_binop("<",  $1, $3); }
     | Expr LE Expr { $$ = make_binop("<=", $1, $3); }
     | Expr EQ Expr { $$ = make_binop("==", $1, $3); }
     | Expr GT Expr { $$ = make_binop(">",  $1, $3); }
     | Expr GE Expr { $$ = make_binop(">=", $1, $3); }
     | Expr NE Expr { $$ = make_binop("!=", $1, $3); }
     | Expr         { $$ = $1; }
     ;

/* 算术表达式（分层，带优先级） */
Expr : Expr ADD Term { $$ = make_binop("+", $1, $3); }
     | Expr SUB Term { $$ = make_binop("-", $1, $3); }
     | Term          { $$ = $1; }
     ;

Term : Term MUL Fact { $$ = make_binop("*", $1, $3); }
     | Term DIV Fact { $$ = make_binop("/", $1, $3); }
     | Fact          { $$ = $1; }
     ;

Fact : NUM                { $$ = make_node(AST_NUM, $1, "int");   }
     | FLO                { $$ = make_node(AST_FLO, $1, "float"); }
     | ID                 { $$ = make_node(AST_ID, $1); }
     | ID LBK Expr RBK    { $$ = make_node(AST_ARRAY_ACCESS, $1); add_child($$, $3); }
     | LPA Expr RPA       { $$ = $2; }
     | ID LPA ArgList RPA { $$ = make_node(AST_CALL, $1); splice($$, $3); }
     ;

/* 实参表：逗号分隔（含尾逗号），可空 */
ArgList : ArgList Arg CMA { $$ = $1; add_child($$, $2); }
        | /* 空 */        { $$ = make_node(AST_ARG_LIST); }
        ;

Arg : Expr        { $$ = $1; }
    | ID LBK RBK  { $$ = make_node(AST_ARRAY_ACCESS, $1); }
    ;

%%

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    if (argc >= 2) {
        yyin = fopen(argv[1], "r");
        if (!yyin) { fprintf(stderr, "无法打开输入文件: %s\n", argv[1]); return 1; }
    }
    if (yyparse() == 0 && g_root) {
        SemanticResult result;
        result.ast_root = g_root;
        analyze(result);                                   // 复用 Lab5 语义分析
        printf("=== AST ===\n");        print_ast(g_root, 0);
        printf("\n=== 符号表 ===\n");     print_symbol_table(result.symbols);
        printf("\n=== 语义检查 ===\n");    print_errors(result.errors);
    }
    return 0;
}
