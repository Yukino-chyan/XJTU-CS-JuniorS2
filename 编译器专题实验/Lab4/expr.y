%{
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif
void yyerror(const char *s) { fprintf(stderr, "错误: %s\n", s); }
int yylex(void);
%}

%token ID

%%
E : E '+' T   { printf("归约: E -> E + T\n"); }
  | T         { printf("归约: E -> T\n"); }
  ;

T : T '*' F   { printf("归约: T -> T * F\n"); }
  | F         { printf("归约: T -> F\n"); }
  ;

F : '(' E ')' { printf("归约: F -> ( E )\n"); }
  | ID        { printf("归约: F -> id\n"); }
  ;
%%

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    printf("请输入表达式（如 id + id * id），回车确认：\n");
    yyparse();
    printf("语法分析完成。\n");
    return 0;
}
