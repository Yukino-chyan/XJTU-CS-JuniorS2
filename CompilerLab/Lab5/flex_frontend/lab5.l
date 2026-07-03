%{
#include <string.h>
#include <stdlib.h>
#include "lab5.tab.h"
%}

%option noyywrap

%%
"//".*                  { /* 行注释，跳过 */ }
[ \t\r\n]+              { /* 空白与换行，跳过 */ }

 /* 关键字（写在标识符规则前面；同长度时靠前的规则优先，更长的输入仍按最长匹配归为 ID） */
"int"                   { return INT; }
"float"                 { return FLOAT; }
"void"                  { return VOID; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"return"                { return RETURN; }
"print"                 { return PRINT; }

 /* 标识符 / 浮点数 / 整数（带词素，建 AST 用） */
[a-zA-Z_][a-zA-Z0-9_]*              { yylval.str = strdup(yytext); return ID;  }
[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?    { yylval.str = strdup(yytext); return FLO; }
[0-9]+                              { yylval.str = strdup(yytext); return NUM; }

 /* 算术运算符 */
"+"                     { return ADD; }
"-"                     { return SUB; }
"*"                     { return MUL; }
"/"                     { return DIV; }

 /* 关系运算符（双字符写在单字符前面） */
"<="                    { return LE; }
">="                    { return GE; }
"=="                    { return EQ; }
"!="                    { return NE; }
"<"                     { return LT; }
">"                     { return GT; }
"="                     { return ASG; }

 /* 界符 */
"("                     { return LPA; }
")"                     { return RPA; }
"["                     { return LBK; }
"]"                     { return RBK; }
"{"                     { return LBR; }
"}"                     { return RBR; }
","                     { return CMA; }
";"                     { return SCO; }

.                       { fprintf(stderr, "词法错误：未知字符 '%s'\n", yytext); }
%%
