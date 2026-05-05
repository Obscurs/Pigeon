# Agent Workflow

## Overview

All feature work is driven by tasks listed in `.claude/docs/PLAN.md`. Each task follows a TDD loop involving BAS (programmer), DIMITRY (QA), and ARTEM (reviewer). The loop repeats until the feature is complete, all tests pass, and ARTEM approves.

---

## TDD Loop — Step by Step

### Step 1 — DIMITRY: Write failing tests

Before any implementation exists, DIMITRY reads the task spec and the module diagrams/info files, then writes the test cases that define the expected behavior. Tests will fail at this point — that is intentional.

DIMITRY writes `DimitryReport.txt` with:
- Which systems have tests, and in which files
- What each test case expects (behavior description, not code)
- Current test results (all failing at this stage)

### Step 2 — BAS: Implement to make tests pass

BAS reads `DimitryReport.txt` (not the test files directly) to understand what behavior is expected, then implements the feature. BAS does not look at UT source files — the report is the interface.

BAS iterates until the test suite passes (DIMITRY runs the tests and reports results back via `DimitryReport.txt`).

BAS writes `BasReport.txt` with:
- What was implemented and why
- Which files were created or modified
- Any assumptions or known edge cases

### Step 3 — ARTEM: Review

ARTEM reads both reports and reviews all changed files — BAS's application/engine code and DIMITRY's test files. ARTEM checks both against the guidelines checklists.

ARTEM writes `ArtemReport.txt` with one of two outcomes:

**APPROVED** — both BAS and DIMITRY changes are correct and the feature is done.

**CHANGES REQUIRED** — a list of blocking issues (must fix) and non-blocking suggestions. Each issue references the exact file and line number and names which agent must fix it (BAS or DIMITRY).

### Step 4 — Iterate

BAS and DIMITRY fix their respective blocking issues and re-run the TDD loop from Step 2 (or Step 1 if test changes are needed). ARTEM re-reviews. This repeats until ARTEM issues an APPROVED verdict.

---

## Report Files

Each agent maintains its own report file in `.claude/docs/`. The file is overwritten each task — it reflects the current task only, not history.

| Agent | File |
|---|---|
| BAS | `.claude/docs/BasReport.txt` |
| DIMITRY | `.claude/docs/DimitryReport.txt` |
| ARTEM | `.claude/docs/ArtemReport.txt` |

Reports are short — a summary, not a log. ARTEM's report is the authoritative record of whether the task is done.

---

## Communication Between Agents

BAS and DIMITRY do not share code directly (BAS cannot read UT files; DIMITRY does not read application/engine files). Their interface is the report files:

- **DIMITRY → BAS**: `DimitryReport.txt` — test expectations and failure output so BAS knows what to implement or fix.
- **BAS → DIMITRY**: `BasReport.txt` — implementation notes so DIMITRY knows what changed and can update tests if needed.
- **Both → ARTEM**: their respective reports plus the actual changed files.
- **ARTEM → Both**: `ArtemReport.txt` — approval or list of blocking issues per agent.

---

## Loop Diagram

```
PLAN.md task
    │
    ▼
DIMITRY: write failing tests → DimitryReport.txt
    │
    ▼
BAS: implement feature → BasReport.txt
    │
    ▼
DIMITRY: run tests, update DimitryReport.txt with results
    │
    ├─ TESTS PASSED ──────────────────────► continue
    │
    └─ TESTS FAILED
           │
           └─ BAS fixes blocking issues and update BasReport.txt
                    └─► back to DIMITRY testing
ARTEM: review all changes → ArtemReport.txt
    │
    ├─ APPROVED ──────────────────────► task complete
    │
    └─ CHANGES REQUIRED
           │
           ├─ BAS fixes blocking issues
           └─ DIMITRY fixes blocking issues
                    │
                    └──────────────────► back to DIMITRY testing
```
