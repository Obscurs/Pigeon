# Agent Workflow

## Overview

All feature work is driven by tasks listed in `.claude/docs/PLAN.md`. Each task follows a TDD loop involving SAB (programmer), DYM (QA), and TEM (reviewer). The loop repeats until the feature is complete, all tests pass, and TEM approves.

---

## TDD Loop — Step by Step

### Step 1 — DYM: Write failing tests

Before any implementation exists, DYM reads the task spec and the module diagrams/info files, then writes the test cases that define the expected behavior. Tests will fail at this point — that is intentional.

DYM writes `DymReport.txt` with:
- Which systems have tests, and in which files
- What each test case expects (behavior description, not code)
- Current test results (all failing at this stage)

### Step 2 — SAB: Implement to make tests pass

SAB reads `DymReport.txt` (not the test files directly) to understand what behavior is expected, then implements the feature. SAB does not look at UT source files — the report is the interface.

SAB iterates until the test suite passes (DYM runs the tests and reports results back via `DymReport.txt`).

SAB writes `SabReport.txt` with:
- What was implemented and why
- Which files were created or modified
- Any assumptions or known edge cases

### Step 3 — TEM: Review

TEM reads both reports and reviews all changed files — SAB's application/engine code and DYM's test files. TEM checks both against the guidelines checklists.

TEM writes `TemReport.txt` with one of two outcomes:

**APPROVED** — both SAB and DYM changes are correct and the feature is done.

**CHANGES REQUIRED** — a list of blocking issues (must fix) and non-blocking suggestions. Each issue references the exact file and line number and names which agent must fix it (SAB or DYM).

### Step 4 — Iterate

SAB and DYM fix their respective blocking issues and re-run the TDD loop from Step 2 (or Step 1 if test changes are needed). TEM re-reviews. This repeats until TEM issues an APPROVED verdict.

---

## Report Files

Each agent maintains its own report file in `.claude/docs/`. The file is overwritten each task — it reflects the current task only, not history.

| Agent | File |
|---|---|
| SAB | `.claude/docs/SabReport.txt` |
| DYM | `.claude/docs/DymReport.txt` |
| TEM | `.claude/docs/TemReport.txt` |

Reports are short — a summary, not a log. TEM's report is the authoritative record of whether the task is done.

---

## Communication Between Agents

SAB and DYM do not share code directly (SAB cannot read UT files; DYM does not read application/engine files). Their interface is the report files:

- **DYM → SAB**: `DymReport.txt` — test expectations and failure output so SAB knows what to implement or fix.
- **SAB → DYM**: `SabReport.txt` — implementation notes so DYM knows what changed and can update tests if needed.
- **Both → TEM**: their respective reports plus the actual changed files.
- **TEM → Both**: `TemReport.txt` — approval or list of blocking issues per agent.

---

## Loop Diagram

```
PLAN.md task
    │
    ▼
DYM: write failing tests → DymReport.txt
    │
    ▼
SAB: implement feature → SabReport.txt
    │
    ▼
DYM: run tests, update DymReport.txt with results
    │
    ├─ TESTS PASSED ──────────────────────► continue
    │
    └─ TESTS FAILED
           │
           └─ SAB fixes blocking issues and update SabReport.txt
                    └─► back to DYM testing
TEM: review all changes → TemReport.txt
    │
    ├─ APPROVED ──────────────────────► task complete
    │
    └─ CHANGES REQUIRED
           │
           ├─ SAB fixes blocking issues
           └─ DYM fixes blocking issues
                    │
                    └──────────────────► back to DYM testing
```
