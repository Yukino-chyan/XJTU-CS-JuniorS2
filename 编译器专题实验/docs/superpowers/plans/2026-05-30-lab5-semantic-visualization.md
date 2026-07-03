# Lab5 Semantic Visualization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a Lab5 semantic-analysis workbench to `CompilerWeb`.

**Architecture:** Build a browser-safe semantic core in `CompilerWeb/semantic.js`, expose it for both browser use and Node tests, then mount the Lab5 UI through the existing `App.register` pattern. Keep UI state local to `mountLab5`.

**Tech Stack:** Static HTML/CSS/JavaScript, D3 already available, Node `node:test` for semantic-core tests.

---

### Task 1: Semantic Core Tests

**Files:**
- Create: `CompilerWeb/tests/semantic-core.test.mjs`
- Modify later: `CompilerWeb/semantic.js`

- [ ] **Step 1: Write failing tests**

Create tests that import `../semantic.js` and assert:

```js
import test from 'node:test';
import assert from 'node:assert/strict';
import SemanticLabCore from '../semantic.js';

test('builds AST and symbol table for a valid function program', () => {
  const source = `int main() {
    int d;
    d = 5;
    return d;
  };
  main()`;

  const result = SemanticLabCore.analyzeSource(source);

  assert.equal(result.diagnostics.length, 0);
  assert.equal(result.ast.type, 'Program');
  assert.equal(result.symbols.some((s) => s.name === 'main' && s.scope === '全局'), true);
  assert.equal(result.symbols.some((s) => s.name === 'd' && s.scope === 'main'), true);
});

test('reports duplicate declarations and type mismatch', () => {
  const source = `int foo() {
    int x;
    int x;
    float y;
    x = y;
    return y;
  };
  foo()`;

  const result = SemanticLabCore.analyzeSource(source);

  assert.equal(result.diagnostics.some((d) => d.message.includes('重复声明：x')), true);
  assert.equal(result.diagnostics.some((d) => d.message.includes('不能把 float 赋给 x(int)')), true);
  assert.equal(result.diagnostics.some((d) => d.message.includes('函数声明返回 int，实际返回 float')), true);
});
```

- [ ] **Step 2: Verify tests fail**

Run: `node --test CompilerWeb/tests/semantic-core.test.mjs`

Expected: FAIL because `CompilerWeb/semantic.js` does not exist or does not export `analyzeSource`.

### Task 2: Semantic Core Implementation

**Files:**
- Create: `CompilerWeb/semantic.js`

- [ ] **Step 1: Implement minimal core**

Implement:

- `scan(source)` for Lab5 keywords, identifiers, numbers, punctuation, and operators.
- Recursive-descent parsing for the demonstration subset: top-level declarations, functions, statements, assignments, returns, calls, arithmetic expressions, and blocks.
- Semantic analysis for scopes, symbols, duplicate declarations, undeclared identifiers, assignment compatibility, and return compatibility.
- `analyzeSource(source)` returning `{ tokens, ast, symbols, diagnostics, stats }`.

- [ ] **Step 2: Verify tests pass**

Run: `node --test CompilerWeb/tests/semantic-core.test.mjs`

Expected: PASS.

### Task 3: Lab5 UI Integration

**Files:**
- Modify: `CompilerWeb/index.html`
- Modify: `CompilerWeb/semantic.js`
- Modify: `CompilerWeb/style.css`

- [ ] **Step 1: Register Lab5**

Add `<script src="semantic.js"></script>` after `slr1.js`. In browser mode, `semantic.js` should call:

```js
App.register({
  id: 'lab5',
  label: '实验五 · 语义分析',
  figNo: '04',
  mount: mountLab5
});
```

- [ ] **Step 2: Build UI**

Inside `mountLab5(root)`, render:

- Left pane: legend, summary, sample buttons, source editor.
- Right pane: AST tree, symbol table, diagnostics, selected-node detail.

- [ ] **Step 3: Add styles**

Add Lab5-specific CSS classes using existing variables and current layout conventions.

### Task 4: Verification

**Files:**
- No new files.

- [ ] **Step 1: Run automated tests**

Run: `node --test CompilerWeb/tests/semantic-core.test.mjs`

Expected: PASS.

- [ ] **Step 2: Static-load check**

Run a lightweight browserless check that loads `index.html` references and confirms `semantic.js` is listed.

- [ ] **Step 3: Manual browser check if available**

Open `CompilerWeb/index.html` or use a static server. Verify Lab5 tab appears, default AST renders, clicking an AST node updates detail, and the error sample shows diagnostics.
