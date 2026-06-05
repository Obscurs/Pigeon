---
name: implement-feature
description: Full TDD + code review loop for the Pigeon project. Implements a feature using red-green-refactor, then reviews it against project architecture and conventions, and iterates until approved. Use when user wants to implement a feature or fix end-to-end, mentions "implement", "build", "add feature", or references a task from PLAN.md.
---

# Implement Feature

Orchestrates the full Pigeon TDD loop inline: **write tests → implement → document → review → fix → repeat**.

Read `.claude/docs/PLAN.md` to identify the active task before starting.

---

## Phase 0 — Grill with docs

Follow the full workflow from [../grill-with-docs/SKILL.md](../grill-with-docs/SKILL.md).

Challenge the feature plan against the domain model, existing terminology, and architectural rules before any implementation begins. Resolve all ambiguities, update `CONTEXT.md` inline as terms are settled, and record hard architectural decisions as ADRs. Do not proceed to Phase 1 until the grilling session is complete.

---

## Phase 1 — TDD (red → green)

Follow the full TDD workflow from [../tdd/SKILL.md](../tdd/SKILL.md).

**Test-first:**
- Read the module `.info` file(s) and `.claude/docs/testing-guidelines.md`
- Write failing tests covering: happy path, guard conditions, event handling, edge cases

**Implementation:**
- Implement the minimum code to make the tests pass (one test at a time)
- Do not read UT files while implementing — infer expected behavior from the tests you just wrote

**Verify:** run the `Testing` build and confirm all tests pass before moving to review.  
Build command: `cmake --build build --config Testing && build\bin\Testing\UT.exe`

If tests fail: fix the implementation and re-run. Repeat until green.

---

## Phase 2 — Documentation

Follow the documentation workflow from [../write-documentation/SKILL.md](../write-documentation/SKILL.md).

Update the `.info` file for every module touched during Phase 1, then update `architecture.md` if any cross-module relationships changed.

---

## Phase 3 — Code review

Follow the full review workflow from [../code-review/SKILL.md](../code-review/SKILL.md).

Apply both checklists from [../code-review/CHECKLIST.md](../code-review/CHECKLIST.md) against the changed files.

Present the verdict inline: `APPROVED` or `CHANGES REQUIRED` with the full issue list.

---

## Phase 4 — Iterate if needed

If verdict is `CHANGES REQUIRED`:

1. Collect all `[BLOCKING]` issues from the review output above
2. Fix implementation issues (engine/app code) and/or test issues as directed
3. Re-run Phase 1 (tests must still pass after fixes)
4. Re-run Phase 2 if the fixes changed any documented behaviour
5. Re-run Phase 3 (new review iteration, increment iteration counter)
6. Repeat until verdict is `APPROVED`

---

## Done

When the review is `APPROVED`, the task is complete. Report to the user with a summary of what was implemented and the final iteration count.
