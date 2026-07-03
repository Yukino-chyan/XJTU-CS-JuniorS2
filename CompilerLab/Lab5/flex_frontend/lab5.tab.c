/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "lab5.y"

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

#line 88 "lab5.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "lab5.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ID = 3,                         /* ID  */
  YYSYMBOL_NUM = 4,                        /* NUM  */
  YYSYMBOL_FLO = 5,                        /* FLO  */
  YYSYMBOL_INT = 6,                        /* INT  */
  YYSYMBOL_FLOAT = 7,                      /* FLOAT  */
  YYSYMBOL_VOID = 8,                       /* VOID  */
  YYSYMBOL_IF = 9,                         /* IF  */
  YYSYMBOL_ELSE = 10,                      /* ELSE  */
  YYSYMBOL_WHILE = 11,                     /* WHILE  */
  YYSYMBOL_RETURN = 12,                    /* RETURN  */
  YYSYMBOL_PRINT = 13,                     /* PRINT  */
  YYSYMBOL_ADD = 14,                       /* ADD  */
  YYSYMBOL_SUB = 15,                       /* SUB  */
  YYSYMBOL_MUL = 16,                       /* MUL  */
  YYSYMBOL_DIV = 17,                       /* DIV  */
  YYSYMBOL_LT = 18,                        /* LT  */
  YYSYMBOL_LE = 19,                        /* LE  */
  YYSYMBOL_EQ = 20,                        /* EQ  */
  YYSYMBOL_GT = 21,                        /* GT  */
  YYSYMBOL_GE = 22,                        /* GE  */
  YYSYMBOL_NE = 23,                        /* NE  */
  YYSYMBOL_ASG = 24,                       /* ASG  */
  YYSYMBOL_LPA = 25,                       /* LPA  */
  YYSYMBOL_RPA = 26,                       /* RPA  */
  YYSYMBOL_LBK = 27,                       /* LBK  */
  YYSYMBOL_RBK = 28,                       /* RBK  */
  YYSYMBOL_LBR = 29,                       /* LBR  */
  YYSYMBOL_RBR = 30,                       /* RBR  */
  YYSYMBOL_CMA = 31,                       /* CMA  */
  YYSYMBOL_SCO = 32,                       /* SCO  */
  YYSYMBOL_YYACCEPT = 33,                  /* $accept  */
  YYSYMBOL_Prog = 34,                      /* Prog  */
  YYSYMBOL_DeclList = 35,                  /* DeclList  */
  YYSYMBOL_Decl = 36,                      /* Decl  */
  YYSYMBOL_Type = 37,                      /* Type  */
  YYSYMBOL_ParamList = 38,                 /* ParamList  */
  YYSYMBOL_Param = 39,                     /* Param  */
  YYSYMBOL_StmtList = 40,                  /* StmtList  */
  YYSYMBOL_Stmt = 41,                      /* Stmt  */
  YYSYMBOL_Bool = 42,                      /* Bool  */
  YYSYMBOL_Expr = 43,                      /* Expr  */
  YYSYMBOL_Term = 44,                      /* Term  */
  YYSYMBOL_Fact = 45,                      /* Fact  */
  YYSYMBOL_ArgList = 46,                   /* ArgList  */
  YYSYMBOL_Arg = 47                        /* Arg  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   125

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  33
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  15
/* YYNRULES -- Number of rules.  */
#define YYNRULES  50
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  106

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   287


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    42,    42,    47,    48,    52,    54,    56,    58,    63,
      64,    65,    69,    70,    73,    74,    78,    79,    80,    84,
      87,    90,    92,    94,    96,    98,   100,   102,   107,   108,
     109,   110,   111,   112,   113,   117,   118,   119,   122,   123,
     124,   127,   128,   129,   130,   131,   132,   136,   137,   140,
     141
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ID", "NUM", "FLO",
  "INT", "FLOAT", "VOID", "IF", "ELSE", "WHILE", "RETURN", "PRINT", "ADD",
  "SUB", "MUL", "DIV", "LT", "LE", "EQ", "GT", "GE", "NE", "ASG", "LPA",
  "RPA", "LBK", "RBK", "LBR", "RBR", "CMA", "SCO", "$accept", "Prog",
  "DeclList", "Decl", "Type", "ParamList", "Param", "StmtList", "Stmt",
  "Bool", "Expr", "Term", "Fact", "ArgList", "Arg", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-32)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -32,    11,     6,   -32,    54,   -32,   -32,   -32,    -4,    22,
      33,    31,    30,    44,    82,    65,   -32,    33,   -32,    33,
      33,    33,    21,   -32,   -32,    33,    -8,    87,   -32,    33,
      39,   -32,    74,    30,    -8,    19,    49,    64,    73,    79,
     -32,    33,    58,    33,    33,    33,    33,    60,   -32,    33,
     -32,   103,   -32,    75,   -32,    -8,    80,    90,    30,    33,
      33,    33,    33,    33,    33,    30,    57,    52,   -32,    87,
      87,   -32,   -32,   -32,    -8,    23,    88,     0,   -32,    33,
     105,    -8,    -8,    -8,    -8,    -8,    -8,   -32,   -32,   -32,
      89,   114,    91,   -32,   -32,    -8,    30,   -32,    92,   -32,
     -32,     6,    93,    76,   -32,   -32
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       4,     0,     0,     1,     0,     9,    10,    11,     0,     0,
       0,     0,     0,     0,     0,     2,    17,     0,    48,     0,
       0,     0,    43,    41,    42,     0,    24,    37,    40,     0,
       0,     3,     5,    18,    19,     0,     0,     0,    34,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    25,     0,
      13,     0,    16,    43,    26,    49,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    45,    35,
      36,    38,    39,    27,     6,     0,     0,     0,    47,     0,
      21,    28,    29,    30,    31,    32,    33,    23,    46,    44,
       0,     0,     0,     7,    50,    20,     0,     4,    14,    12,
      22,     0,     0,     0,    15,     8
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -32,   -32,    25,   -32,    45,   -32,   -32,   -12,   -31,   104,
      -9,    66,    67,    84,   -32
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,    13,    14,    75,    92,    15,    16,    37,
      38,    27,    28,    35,    56
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      30,    26,    52,    22,    23,    24,    43,    44,    34,     4,
      36,     3,     5,     6,     7,     8,    42,     9,    10,    11,
      47,    20,    53,    23,    24,    25,    55,    80,    94,     5,
       6,     7,    67,     4,    87,    12,    22,    23,    24,     8,
      74,     9,    10,    11,    25,    54,    40,    21,    41,    90,
      81,    82,    83,    84,    85,    86,    29,    55,    25,    12,
      53,    23,    24,    43,    44,   100,    43,    44,    67,    48,
      95,    33,    43,    44,    43,    44,    31,    57,    17,    18,
      89,    19,    25,    88,    68,    32,    73,    43,    44,   103,
      58,    59,    60,    61,    62,    63,    64,    33,    49,    50,
      40,    51,    77,    45,    46,    65,   105,    76,    33,    69,
      70,    78,    71,    72,    79,    96,    93,    98,    97,   102,
      91,   104,   101,    99,    66,    39
};

static const yytype_int8 yycheck[] =
{
      12,    10,    33,     3,     4,     5,    14,    15,    17,     3,
      19,     0,     6,     7,     8,     9,    25,    11,    12,    13,
      29,    25,     3,     4,     5,    25,    35,    58,    28,     6,
       7,     8,    41,     3,    65,    29,     3,     4,     5,     9,
      49,    11,    12,    13,    25,    26,    25,    25,    27,    26,
      59,    60,    61,    62,    63,    64,    25,    66,    25,    29,
       3,     4,     5,    14,    15,    96,    14,    15,    77,    30,
      79,    32,    14,    15,    14,    15,    32,    28,    24,    25,
      28,    27,    25,    26,    26,     3,    26,    14,    15,   101,
      26,    18,    19,    20,    21,    22,    23,    32,    24,    25,
      25,    27,    27,    16,    17,    26,    30,     4,    32,    43,
      44,    31,    45,    46,    24,    10,    28,     3,    29,    27,
      75,    28,    97,    32,    40,    21
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    34,    35,     0,     3,     6,     7,     8,     9,    11,
      12,    13,    29,    36,    37,    40,    41,    24,    25,    27,
      25,    25,     3,     4,     5,    25,    43,    44,    45,    25,
      40,    32,     3,    32,    43,    46,    43,    42,    43,    42,
      25,    27,    43,    14,    15,    16,    17,    43,    30,    24,
      25,    27,    41,     3,    26,    43,    47,    28,    26,    18,
      19,    20,    21,    22,    23,    26,    46,    43,    26,    44,
      44,    45,    45,    26,    43,    38,     4,    27,    31,    24,
      41,    43,    43,    43,    43,    43,    43,    41,    26,    28,
      26,    37,    39,    28,    28,    43,    10,    29,     3,    32,
      41,    35,    27,    40,    28,    30
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    33,    34,    35,    35,    36,    36,    36,    36,    37,
      37,    37,    38,    38,    39,    39,    40,    40,    40,    41,
      41,    41,    41,    41,    41,    41,    41,    41,    42,    42,
      42,    42,    42,    42,    42,    43,    43,    43,    44,    44,
      44,    45,    45,    45,    45,    45,    45,    46,    46,    47,
      47
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     3,     0,     2,     4,     5,     9,     1,
       1,     1,     3,     0,     2,     4,     3,     1,     2,     3,
       6,     5,     7,     5,     2,     3,     4,     4,     3,     3,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       1,     1,     1,     1,     4,     3,     4,     3,     0,     1,
       3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* Prog: DeclList StmtList  */
#line 43 "lab5.y"
         { g_root = make_node(AST_PROGRAM); splice(g_root, (yyvsp[-1].node)); splice(g_root, (yyvsp[0].node)); }
#line 1186 "lab5.tab.c"
    break;

  case 3: /* DeclList: DeclList Decl SCO  */
#line 47 "lab5.y"
                              { (yyval.node) = (yyvsp[-2].node); add_child((yyval.node), (yyvsp[-1].node)); }
#line 1192 "lab5.tab.c"
    break;

  case 4: /* DeclList: %empty  */
#line 48 "lab5.y"
                               { (yyval.node) = make_node(AST_DECL_LIST); }
#line 1198 "lab5.tab.c"
    break;

  case 5: /* Decl: Type ID  */
#line 53 "lab5.y"
         { (yyval.node) = make_node(AST_VAR_DECL, (yyvsp[0].str), (yyvsp[-1].node)->val_type.c_str()); }
#line 1204 "lab5.tab.c"
    break;

  case 6: /* Decl: Type ID ASG Expr  */
#line 55 "lab5.y"
         { (yyval.node) = make_node(AST_VAR_DECL, (yyvsp[-2].str), (yyvsp[-3].node)->val_type.c_str()); add_child((yyval.node), (yyvsp[0].node)); }
#line 1210 "lab5.tab.c"
    break;

  case 7: /* Decl: Type ID LBK NUM RBK  */
#line 57 "lab5.y"
         { (yyval.node) = make_node(AST_VAR_DECL, (yyvsp[-3].str), ((yyvsp[-4].node)->val_type + "[]").c_str()); }
#line 1216 "lab5.tab.c"
    break;

  case 8: /* Decl: Type ID LPA ParamList RPA LBR DeclList StmtList RBR  */
#line 59 "lab5.y"
         { (yyval.node) = make_node(AST_FUN_DECL, (yyvsp[-7].str), (yyvsp[-8].node)->val_type.c_str());
           splice((yyval.node), (yyvsp[-5].node)); splice((yyval.node), (yyvsp[-2].node)); splice((yyval.node), (yyvsp[-1].node)); }
#line 1223 "lab5.tab.c"
    break;

  case 9: /* Type: INT  */
#line 63 "lab5.y"
              { (yyval.node) = make_node(AST_TYPE, "int",   "int");   }
#line 1229 "lab5.tab.c"
    break;

  case 10: /* Type: FLOAT  */
#line 64 "lab5.y"
              { (yyval.node) = make_node(AST_TYPE, "float", "float"); }
#line 1235 "lab5.tab.c"
    break;

  case 11: /* Type: VOID  */
#line 65 "lab5.y"
              { (yyval.node) = make_node(AST_TYPE, "void",  "void");  }
#line 1241 "lab5.tab.c"
    break;

  case 12: /* ParamList: ParamList Param SCO  */
#line 69 "lab5.y"
                                { (yyval.node) = (yyvsp[-2].node); add_child((yyval.node), (yyvsp[-1].node)); }
#line 1247 "lab5.tab.c"
    break;

  case 13: /* ParamList: %empty  */
#line 70 "lab5.y"
                                 { (yyval.node) = make_node(AST_PARAM_LIST); }
#line 1253 "lab5.tab.c"
    break;

  case 14: /* Param: Type ID  */
#line 73 "lab5.y"
                         { (yyval.node) = make_node(AST_PARAM, (yyvsp[0].str), (yyvsp[-1].node)->val_type.c_str()); }
#line 1259 "lab5.tab.c"
    break;

  case 15: /* Param: Type ID LBK RBK  */
#line 74 "lab5.y"
                         { (yyval.node) = make_node(AST_PARAM, (yyvsp[-2].str), ((yyvsp[-3].node)->val_type + "[]").c_str()); }
#line 1265 "lab5.tab.c"
    break;

  case 16: /* StmtList: StmtList SCO Stmt  */
#line 78 "lab5.y"
                             { (yyval.node) = (yyvsp[-2].node); add_child((yyval.node), (yyvsp[0].node)); }
#line 1271 "lab5.tab.c"
    break;

  case 17: /* StmtList: Stmt  */
#line 79 "lab5.y"
                             { (yyval.node) = make_node(AST_STMT_LIST); add_child((yyval.node), (yyvsp[0].node)); }
#line 1277 "lab5.tab.c"
    break;

  case 18: /* StmtList: StmtList SCO  */
#line 80 "lab5.y"
                             { (yyval.node) = (yyvsp[-1].node); }
#line 1283 "lab5.tab.c"
    break;

  case 19: /* Stmt: ID ASG Expr  */
#line 85 "lab5.y"
         { ASTNode* t = make_node(AST_ID, (yyvsp[-2].str));
           (yyval.node) = make_node(AST_ASSIGN); add_child((yyval.node), t); add_child((yyval.node), (yyvsp[0].node)); }
#line 1290 "lab5.tab.c"
    break;

  case 20: /* Stmt: ID LBK Expr RBK ASG Expr  */
#line 88 "lab5.y"
         { ASTNode* t = make_node(AST_ARRAY_ACCESS, (yyvsp[-5].str)); add_child(t, (yyvsp[-3].node));
           (yyval.node) = make_node(AST_ASSIGN); add_child((yyval.node), t); add_child((yyval.node), (yyvsp[0].node)); }
#line 1297 "lab5.tab.c"
    break;

  case 21: /* Stmt: IF LPA Bool RPA Stmt  */
#line 91 "lab5.y"
         { (yyval.node) = make_node(AST_IF_STMT); add_child((yyval.node), (yyvsp[-2].node)); add_child((yyval.node), (yyvsp[0].node)); }
#line 1303 "lab5.tab.c"
    break;

  case 22: /* Stmt: IF LPA Bool RPA Stmt ELSE Stmt  */
#line 93 "lab5.y"
         { (yyval.node) = make_node(AST_IF_STMT); add_child((yyval.node), (yyvsp[-4].node)); add_child((yyval.node), (yyvsp[-2].node)); add_child((yyval.node), (yyvsp[0].node)); }
#line 1309 "lab5.tab.c"
    break;

  case 23: /* Stmt: WHILE LPA Bool RPA Stmt  */
#line 95 "lab5.y"
         { (yyval.node) = make_node(AST_WHILE_STMT); add_child((yyval.node), (yyvsp[-2].node)); add_child((yyval.node), (yyvsp[0].node)); }
#line 1315 "lab5.tab.c"
    break;

  case 24: /* Stmt: RETURN Expr  */
#line 97 "lab5.y"
         { (yyval.node) = make_node(AST_RETURN_STMT); add_child((yyval.node), (yyvsp[0].node)); }
#line 1321 "lab5.tab.c"
    break;

  case 25: /* Stmt: LBR StmtList RBR  */
#line 99 "lab5.y"
         { (yyval.node) = make_node(AST_COMP_STMT); splice((yyval.node), (yyvsp[-1].node)); }
#line 1327 "lab5.tab.c"
    break;

  case 26: /* Stmt: ID LPA ArgList RPA  */
#line 101 "lab5.y"
         { (yyval.node) = make_node(AST_CALL, (yyvsp[-3].str)); splice((yyval.node), (yyvsp[-1].node)); }
#line 1333 "lab5.tab.c"
    break;

  case 27: /* Stmt: PRINT LPA Expr RPA  */
#line 103 "lab5.y"
         { (yyval.node) = make_node(AST_PRINT_STMT); add_child((yyval.node), (yyvsp[-1].node)); }
#line 1339 "lab5.tab.c"
    break;

  case 28: /* Bool: Expr LT Expr  */
#line 107 "lab5.y"
                    { (yyval.node) = make_binop("<",  (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1345 "lab5.tab.c"
    break;

  case 29: /* Bool: Expr LE Expr  */
#line 108 "lab5.y"
                    { (yyval.node) = make_binop("<=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1351 "lab5.tab.c"
    break;

  case 30: /* Bool: Expr EQ Expr  */
#line 109 "lab5.y"
                    { (yyval.node) = make_binop("==", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1357 "lab5.tab.c"
    break;

  case 31: /* Bool: Expr GT Expr  */
#line 110 "lab5.y"
                    { (yyval.node) = make_binop(">",  (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1363 "lab5.tab.c"
    break;

  case 32: /* Bool: Expr GE Expr  */
#line 111 "lab5.y"
                    { (yyval.node) = make_binop(">=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1369 "lab5.tab.c"
    break;

  case 33: /* Bool: Expr NE Expr  */
#line 112 "lab5.y"
                    { (yyval.node) = make_binop("!=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1375 "lab5.tab.c"
    break;

  case 34: /* Bool: Expr  */
#line 113 "lab5.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 1381 "lab5.tab.c"
    break;

  case 35: /* Expr: Expr ADD Term  */
#line 117 "lab5.y"
                     { (yyval.node) = make_binop("+", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1387 "lab5.tab.c"
    break;

  case 36: /* Expr: Expr SUB Term  */
#line 118 "lab5.y"
                     { (yyval.node) = make_binop("-", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1393 "lab5.tab.c"
    break;

  case 37: /* Expr: Term  */
#line 119 "lab5.y"
                     { (yyval.node) = (yyvsp[0].node); }
#line 1399 "lab5.tab.c"
    break;

  case 38: /* Term: Term MUL Fact  */
#line 122 "lab5.y"
                     { (yyval.node) = make_binop("*", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1405 "lab5.tab.c"
    break;

  case 39: /* Term: Term DIV Fact  */
#line 123 "lab5.y"
                     { (yyval.node) = make_binop("/", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1411 "lab5.tab.c"
    break;

  case 40: /* Term: Fact  */
#line 124 "lab5.y"
                     { (yyval.node) = (yyvsp[0].node); }
#line 1417 "lab5.tab.c"
    break;

  case 41: /* Fact: NUM  */
#line 127 "lab5.y"
                          { (yyval.node) = make_node(AST_NUM, (yyvsp[0].str), "int");   }
#line 1423 "lab5.tab.c"
    break;

  case 42: /* Fact: FLO  */
#line 128 "lab5.y"
                          { (yyval.node) = make_node(AST_FLO, (yyvsp[0].str), "float"); }
#line 1429 "lab5.tab.c"
    break;

  case 43: /* Fact: ID  */
#line 129 "lab5.y"
                          { (yyval.node) = make_node(AST_ID, (yyvsp[0].str)); }
#line 1435 "lab5.tab.c"
    break;

  case 44: /* Fact: ID LBK Expr RBK  */
#line 130 "lab5.y"
                          { (yyval.node) = make_node(AST_ARRAY_ACCESS, (yyvsp[-3].str)); add_child((yyval.node), (yyvsp[-1].node)); }
#line 1441 "lab5.tab.c"
    break;

  case 45: /* Fact: LPA Expr RPA  */
#line 131 "lab5.y"
                          { (yyval.node) = (yyvsp[-1].node); }
#line 1447 "lab5.tab.c"
    break;

  case 46: /* Fact: ID LPA ArgList RPA  */
#line 132 "lab5.y"
                          { (yyval.node) = make_node(AST_CALL, (yyvsp[-3].str)); splice((yyval.node), (yyvsp[-1].node)); }
#line 1453 "lab5.tab.c"
    break;

  case 47: /* ArgList: ArgList Arg CMA  */
#line 136 "lab5.y"
                          { (yyval.node) = (yyvsp[-2].node); add_child((yyval.node), (yyvsp[-1].node)); }
#line 1459 "lab5.tab.c"
    break;

  case 48: /* ArgList: %empty  */
#line 137 "lab5.y"
                           { (yyval.node) = make_node(AST_ARG_LIST); }
#line 1465 "lab5.tab.c"
    break;

  case 49: /* Arg: Expr  */
#line 140 "lab5.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 1471 "lab5.tab.c"
    break;

  case 50: /* Arg: ID LBK RBK  */
#line 141 "lab5.y"
                  { (yyval.node) = make_node(AST_ARRAY_ACCESS, (yyvsp[-2].str)); }
#line 1477 "lab5.tab.c"
    break;


#line 1481 "lab5.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 144 "lab5.y"


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
