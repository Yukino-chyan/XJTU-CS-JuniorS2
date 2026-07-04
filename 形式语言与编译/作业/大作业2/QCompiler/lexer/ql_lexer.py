"""QL 词法分析器：手写 DFA 状态机扫描，将 QL 源程序切分为 Token 序列。

扫描主循环本身就是一个显式有限状态自动机（DFA）：
    START 状态：跳过空白与 '//' 行注释；再按"首字符"分派到各子自动机：
        [a-zA-Z]      -> 标识符/关键字子 DFA（吃光字母后查关键字表）
        [0-9]         -> 整常量子 DFA（吃光数字）
        运算符首字符  -> 运算符子 DFA（最长匹配，前看一位决定是否两字符）
        界符          -> 单字符直接产出
        其它          -> 词法错误（带行列号）
每识别出一个 token 即回到 START，体现 DFA 的"接受即重启"。

对外接口：
    Lexer(source).tokenize() -> list[Token]
该接口为纯内存调用，步骤2 的语法分析器可直接复用，无需经过文件。
"""

from .tokens import Token, TokenType, KEYWORDS, SINGLE_DELIMITERS, OPERATORS


class LexError(Exception):
    """词法错误：携带行列号，便于报告展示与后续阶段诊断。"""

    def __init__(self, message, line, col):
        super().__init__(f"词法错误 (行 {line}, 列 {col}): {message}")
        self.line = line
        self.col = col


def _is_letter(ch):
    return ch is not None and (("a" <= ch <= "z") or ("A" <= ch <= "Z"))


def _is_digit(ch):
    return ch is not None and ("0" <= ch <= "9")


class Lexer:
    """QL 词法分析器。一次构造对应一份源程序。"""

    def __init__(self, source):
        self.src = source
        self.pos = 0          # 当前读取位置（字符下标）
        self.line = 1         # 当前行号（从 1 计）
        self.col = 1          # 当前列号（从 1 计）
        self.tokens = []

    # ———————————————— 字符级辅助 ————————————————
    def _peek(self, offset=0):
        """前看：返回 pos+offset 处字符，越界返回 None。"""
        i = self.pos + offset
        return self.src[i] if i < len(self.src) else None

    def _advance(self):
        """消费当前字符，维护行列号，返回该字符。"""
        ch = self.src[self.pos]
        self.pos += 1
        if ch == "\n":
            self.line += 1
            self.col = 1
        else:
            self.col += 1
        return ch

    def _emit(self, ttype, value, line, col):
        self.tokens.append(Token(ttype, value, line, col))

    # ———————————————— 主循环（START 状态）————————————————
    def tokenize(self):
        """执行词法分析，返回 Token 列表（末尾含一个 EOF）。"""
        while self.pos < len(self.src):
            ch = self._peek()

            # 跳过空白
            if ch in (" ", "\t", "\r", "\n"):
                self._advance()
                continue

            # 跳过 '//' 行注释
            if ch == "/" and self._peek(1) == "/":
                self._skip_line_comment()
                continue

            line, col = self.line, self.col  # 记录 token 起始位置

            if _is_letter(ch):
                self._scan_identifier(line, col)
            elif _is_digit(ch):
                self._scan_number(line, col)
            elif ch in OPERATORS:
                self._scan_operator(line, col)
            elif ch in SINGLE_DELIMITERS:
                self._advance()
                self._emit(SINGLE_DELIMITERS[ch], ch, line, col)
            else:
                raise LexError(f"非法字符 {ch!r}", line, col)

        self._emit(TokenType.EOF, "EOF", self.line, self.col)
        return self.tokens

    # ———————————————— 各子自动机 ————————————————
    def _skip_line_comment(self):
        """已知当前为 '//'，吃到行尾（换行留给主循环处理）。"""
        while self._peek() is not None and self._peek() != "\n":
            self._advance()

    def _scan_identifier(self, line, col):
        """标识符/关键字子 DFA：[a-zA-Z]+ ，结束后查关键字表。"""
        chars = []
        while _is_letter(self._peek()):
            chars.append(self._advance())
        lexeme = "".join(chars)
        ttype = KEYWORDS.get(lexeme, TokenType.ID)
        self._emit(ttype, lexeme, line, col)

    def _scan_number(self, line, col):
        """整常量子 DFA：[0-9]+ 。"""
        chars = []
        while _is_digit(self._peek()):
            chars.append(self._advance())
        self._emit(TokenType.NUM, "".join(chars), line, col)

    def _scan_operator(self, line, col):
        """运算符子 DFA：最长匹配——先试两字符规则，再退回单字符默认规则。"""
        first = self._advance()
        rules = OPERATORS[first]
        nxt = self._peek()

        # 1) 优先匹配需要第二个字符的规则（最长匹配）
        for expected, ttype in rules:
            if expected is not None and nxt == expected:
                second = self._advance()
                self._emit(ttype, first + second, line, col)
                return

        # 2) 退回到单字符默认规则
        for expected, ttype in rules:
            if expected is None:
                self._emit(ttype, first, line, col)
                return

        # 3) 无默认规则（如单独的 & 或 |）→ 词法错误
        need = rules[0][0]
        raise LexError(
            f"非法运算符 {first!r}（期望 {first}{need}）", line, col
        )
