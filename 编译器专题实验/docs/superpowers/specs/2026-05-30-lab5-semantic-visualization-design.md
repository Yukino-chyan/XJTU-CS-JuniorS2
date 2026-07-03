# Lab5 Semantic Visualization Design

## Goal

Implement Lab5 as a semantic-analysis workbench in `CompilerWeb`, following the existing paper-like visual style and keeping the view focused on semantic results rather than parser internals.

## Selected Approach

Use方案 A: 语义结果工作台.

The Lab5 page should show source input on the left and semantic results on the right:

- Token stream summary generated from the source program.
- AST tree as the main visual surface.
- Symbol table grouped by scope.
- Semantic diagnostics, especially duplicate declarations, undeclared identifiers, assignment type mismatch, and return type mismatch.

Parser step-by-step playback is intentionally deferred to Lab6 or later, where the display can become more complex.

## UI Structure

The page follows the current `CompilerWeb` pattern:

- Register a new `lab5` module via `App.register`.
- Use the existing two-column `.lab-grid`.
- Left pane contains legend, summary, and editable source program.
- Right pane contains a Lab5 header, a compact toolbar for view mode, AST visualization, and a lower detail/result area.

The visual style should reuse the current CSS variables, typography, dashed section titles, muted paper background, rust highlights, sage success state, and JetBrains Mono for code-like content.

## Behavior

The page runs entirely in the browser as a static app:

- Editing source automatically regenerates tokens, AST, symbols, and diagnostics.
- Clicking an AST node highlights it and shows node details.
- The default sample should include a valid function and call so the initial page is not error-heavy.
- A small sample selector may include an error sample for demonstrating diagnostics.

## Scope

In scope:

- A JavaScript semantic core sufficient for Lab5 demonstration examples.
- AST, symbol table, diagnostics rendering.
- Registration in navigation and `index.html`.
- Tests for the semantic core.

Out of scope:

- Running the native `semantic.exe` from the browser.
- Full C language support.
- SLR parser playback timeline.
- IR generation or Lab6 intermediate-code visualization.

## Verification

- Node-based tests cover scanner/parser/analyzer behavior.
- Static page loads without console errors.
- Lab5 tab appears and renders the default sample.
