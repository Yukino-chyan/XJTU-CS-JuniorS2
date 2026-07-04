# QCompiler —— QL 语言简易编译器（大作业2 / B 路线）

本项目是形式语言与编译大作业2 的简易编译器实现
统一测试用例为快速排序程序 `qsort`。

## 一、运行环境

- 编译器本体：Python 3，在 PC 上开发运行。
- 目标程序平台：华为鲲鹏服务器，工具链 `as` / `ld` / `gdb`。
- 注意：请用 **`py`** 启动器运行各脚本。

## 二、目录结构

```
QCompiler/
├── lexer/                词法分析
│   ├── tokens.py           Token 定义与词法规则表（数据驱动）
│   └── ql_lexer.py         Lexer：DFA 状态机扫描
├── parser/               语法分析（SLR(1) 表驱动）+ QAST
│   ├── grammar.txt         改造后的 QL 文法（SLR(1) 版，终结符=token 名）
│   ├── slr1_table.txt      SLR(1) 分析表（由 slr1.exe 读 grammar.txt 生成，分析器读取）
│   ├── slr1_report.txt     SLR 构造完整输出（项目集 / FIRST-FOLLOW / 分析表 / 冲突）
│   ├── slr_table.py        读取 slr1_table.txt
│   ├── qast.py             QAST 节点（通用节点 + kind 标签）
│   ├── semantic.py         语义动作：归约时建 QAST
│   └── ql_parser.py        表驱动移进-归约分析器
├── irgen/                中间代码生成（遍历 QAST → QTAC）
│   ├── symtab.py           符号表
│   ├── qtac.py             QTAC 指令表示与文本输出
│   └── codegen.py          遍历 QAST 生成 QTAC（语法制导）
├── armgen/               目标代码生成（QTAC → ARM64）
│   ├── qtac_ir.py          σ-DFA 识别 QTAC 文本为结构化指令
│   ├── emitter.py          ARM 指令文本发射（Q2ARM 模板）
│   └── codegen.py          栈槽分配 + 模板映射，生成 ARM64
├── optimize/             代码优化
│   ├── blocks.py           基本块划分
│   ├── passes.py           公共子表达式消除 / 复制传播 / 死代码删除
│   ├── peephole.py         冗余跳转消除
│   └── qtac_text.py        优化后 QTAC 序列化为文本
├── samples/
│   └── qsort.ql            测试用例：快速排序 QL 源码
├── out/                  各阶段输出（详见第四节"提交物对照"）
├── run_lexer.py          驱动：QL 源 → Token
├── run_parser.py         驱动：QL 源 → QAST
├── run_codegen.py        驱动：QL 源 → QTAC + 符号表
├── run_arm.py            驱动：QTAC → ARM64
├── run_opt.py            驱动：QTAC/ARM 优化
└── slr1.exe              SLR(1) 分析表生成器（复用实验四 slr1.cpp 编译所得）
```

## 三、运行方式

各阶段由对应驱动脚本运行（默认输入 `samples/qsort.ql`，输出到 `out/`）：

```bash
py run_lexer.py      # 词法分析     → out/tokens.txt
py run_parser.py     # 语法分析     → out/ast.txt（QAST）
py run_codegen.py    # 中间代码生成 → out/qsort.qtac、out/symtab.txt
py run_arm.py        # 目标代码生成 → out/qsort.s
py run_opt.py        # 代码优化     → out/qsort_opt.qtac、out/qsort_opt.s
```

SLR(1) 分析表已生成好 `slr1_table.txt`

## 四、指导书要求提交物对照

| 指导书要求的输出物 | 对应文件 |
|---|---|
| 词法分析器源码 + Token 输出文件 | `lexer/`、`run_lexer.py` + **`out/tokens.txt`** |
| SLR(1) 分析器源码 + QAST 文件（B 路线） | `parser/`（`grammar.txt`、`slr_table.py`、`ql_parser.py`、`slr1_table.txt`；+ **`out/ast.txt`** |
| 遍历 QAST 的语义分析及 QTAC 生成源码 | `parser/qast.py`、`parser/semantic.py`（建 QAST）+ `irgen/`（遍历生成 QTAC）、`run_codegen.py` |
| QTAC 中间代码及符号表文件 | **`out/qsort.qtac`**、**`out/symtab.txt`** |
| σ-DFA 转 QTAC 为 ARM 代码的程序源码 | `armgen/`（`qtac_ir.py`、`emitter.py`、`codegen.py`）、`run_arm.py` |
| 未优化 ARM 汇编文件 | **`out/qsort.s`** |
| 优化后 ARM 汇编文件及各优化遍源码 | **`out/qsort_opt.s`**、`optimize/` + 优化后中间代码 **`out/qsort_opt.qtac`** |
| 鲲鹏平台可执行文件 + 运行截图 | **`out/qsort_verify.s`**（已初始化测试数据，在鲲鹏 `as`/`ld` 得可执行文件）；运行/gdb 截图见实验报告 |
| 实验报告 | 随附件单独提交 |
