---
name: write-documentation
description: Reads source code deeply and writes or updates module documentation in Documentation/diagrams/<ModuleName>.info, then updates architecture.md with the module's systems and inter-module relationships. Use when user asks to "document", "write docs for", "update documentation for" a system or module, or after new systems are added.
---

# Write Documentation

Read code deeply and produce accurate documentation. Never guess — every claim must be traceable to source.

---

## Step 1 — Identify scope

Determine which module is being documented. A module maps to a folder:

- `Code/PigeonLib/Pigeon/<ModuleName>/` for engine modules
- `Code/SandboxApp/Sandbox/` for the sandbox module

Collect all files in scope: every `*System.h`, `*System.cpp`, `*Component.h`, `*Event.h`, and `*SingletonComponent.h` in the folder.

---

## Step 2 — Read the code

Read every file in scope. For each system, extract:

- **What it reads** — components and events in `DeclareAccess()` declared as read
- **What it writes/adds** — components and events declared as write, add, inframe-add, or emplace-event
- **What it does** — logic in `Update()`, summarised as behaviour, not code
- **Execution order constraints** — implicit from the DeclareAccess read/write relationships

Also note:
- Which components are defined in this module vs. imported from another
- Which events this module emits or consumes from outside

---

## Step 3 — Write the `.info` file

Write to `Documentation/diagrams/<ModuleName>.info`. If the file exists, update it in place — do not discard existing content unless it is wrong.

Use this format exactly:

```
# <ModuleName> Module

## Purpose
<One paragraph: what problem this module solves and where it sits in the architecture.>

## Systems

### <SystemName>
File: <relative path from repo root>
Reads: <comma-separated list of components/events, or "none">
Writes: <comma-separated list of components/events, or "none">
Role: <one sentence — what this system does each frame>

(repeat for each system in the module)

## Data Flow
<Prose description of how data moves through the systems in this module in execution order.
Include which system produces each key component and which consumes it.>

## Inter-Module Dependencies
Depends on: <components or events imported from other modules, with their source module>
Exposes: <components or events this module produces that other modules consume>
```

---

## Step 4 — Update architecture.md

Open `.claude/docs/architecture.md` and update the **SandboxApp Domain Modules** section.

For each module you've documented, ensure there is an entry. Add one if missing; update if stale. Keep entries short — one or two sentences per module covering its purpose and the systems it contains. Do not rewrite sections unrelated to the module you just documented.

If the module introduces a relationship with another module not yet captured anywhere in `architecture.md`, add a note under a `## Cross-Module Data Flow` section (create it if absent).

---

## Done

Report to the user:
- Which `.info` file was written or updated
- Which sections of `architecture.md` changed
- Any inconsistencies found between the code and existing documentation
