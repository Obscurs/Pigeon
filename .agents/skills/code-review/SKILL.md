---
name: code-review
description: Pigeon project code review against ECS architecture, coding conventions, and test coverage rules. Use after a feature or fix is implemented to verify it follows project architecture, does not introduce duplication, and tests are correct. Use when user says "review", "code review", or asks to verify a completed implementation.
---

# Code Review

Verify that the implementation and tests meet Pigeon's architecture and conventions. You are strictly **read-only** — never modify any source file.

## Before You Start

Read these files:

1. `.claude/docs/architecture.md` — ECS hard rules and project split
2. `.claude/docs/coding-guidelines.md` — naming, component/system/event conventions
3. `.claude/docs/testing-guidelines.md` — test structure and coverage rules
4. `Documentation/diagrams/<ModuleName>.info` for each module touched
5. The changed files themselves — use `git diff` to identify what to review

## Review

Apply both checklists from [CHECKLIST.md](CHECKLIST.md):

- **Application/Engine code** — architecture, components, systems, events, naming, general
- **Test code** — file structure, isolation, naming, coverage

For each issue found, note:
- `[BLOCKING]` — must be fixed before approval
- `[SUGGESTION]` — non-blocking improvement

Label each issue as **implementation** (engine/app code) or **tests**.

## Output

Present the review verdict directly in the conversation:

```
Iteration: <number>
Verdict: APPROVED | CHANGES REQUIRED

--- Implementation (application/engine) ---
<APPROVED | list of issues>
Each issue: [BLOCKING|SUGGESTION] <file:line> — <description>

--- Tests ---
<APPROVED | list of issues>
Each issue: [BLOCKING|SUGGESTION] <file:line> — <description>

Summary:
<1-2 sentences on what was verified or what must change>
```

Re-review after each fix iteration until all blocking issues are resolved, then issue `APPROVED`.
