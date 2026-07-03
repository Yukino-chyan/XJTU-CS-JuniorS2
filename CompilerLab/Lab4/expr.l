%{
#include "expr.tab.h"
%}

%%
[a-zA-Z_][a-zA-Z0-9_]*   { return ID; }
[0-9]+                    { return ID; }
"+"                       { return '+'; }
"*"                       { return '*'; }
"("                       { return '('; }
")"                       { return ')'; }
[ \t\r]                   { /* 跳过空白 */ }
\n                        { return 0; }  /* 回车 = 输入结束，通知 bison EOF */
.                         { printf("未知字符: %s\n", yytext); }
%%

int yywrap(void) { return 1; }
