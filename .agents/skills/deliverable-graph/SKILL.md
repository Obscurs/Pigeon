---
name: deliverable-graph
description: Build and maintain a dependency graph of feature deliverables, rendered as a color-coded Mermaid diagram with implementation order. Use when the user types a feature/deliverable to map, wants to explore or modify deliverables, asks about feature dependencies, a dependency diagram, a feature map, or what to build first.
---

# Deliverable Graph

Maintain a dependency graph of **deliverables** in `Deliverables/` (one markdown file per node) and a generated Mermaid view in `Deliverables/_graph.md`. You propose requirements; the user judges. Status is always recomputed by hand, never stored.

## Glossary

- **Deliverable** = **feature**. The only node type — a capability phrased as an outcome ("player can move around the map").
- **Requirement** = **dependency**. The only edge type. "A requires B" = edge A → B = "B must exist for A."
- **Resolved (local):** node has no `[open]` requirements.
- **Complete (transitive):** resolved AND every linked dependency is itself complete. A resolved leaf is complete.
- **Incomplete:** an open requirement exists somewhere in the subtree.

This vocabulary is the tool's own. Do **not** add it to engine `CONTEXT.md`.

## File format — `Deliverables/<slug>.md`

```markdown
---
id: player-move-map          # stable key; never changes on retitle. Slug derives the filename.
title: Player can move around the map
clear_enough: false          # true = whole node declared trivial; skip requirement tracking
---

## Statement
Player can move around the map.

## Requirements
- [linked] map definition → 2d-tile-map
- [inline] movement input — WASD translates the player transform
- [clear]  coordinate system — standard screen pixels
- [open]   collision — what stops the player at map edges?

## Detail
(prose backing any [inline] requirements)
```

Requirement states: `[linked] <aspect> → <id>` (edge), `[inline]` (satisfied by Detail prose), `[clear]` (trivial/known), `[open]` (accepted but unresolved). `depends_on` is **derived** from `[linked]` lines — never store it separately. Never store status in the node file.

## Status algorithm (recompute from scratch every change)

1. **Build edges:** for every node, edges = the `→ id` targets of its `[linked]` requirements.
2. **Detect cycles:** if the graph has a dependency loop, STOP, report the loop, fix nothing else.
3. **resolved(n):** true iff `clear_enough` OR the node has zero `[open]` requirements.
4. **complete(n):** `resolved(n)` AND every linked target is `complete`. Compute via post-order / fixpoint; a resolved node with no links is complete.
5. **Color each node:**
   - **green** — complete.
   - **red** — NOT resolved (has `[open]` requirements). This is where work/decisions are needed.
   - **amber** — resolved but not complete (a descendant is red).
6. **Build order:** topological sort with dependencies first. Leaves (no outgoing edges) are step 1; goals (nothing points to them) are last. Break ties alphabetically by id.

## Render — `Deliverables/_graph.md`

```markdown
# Deliverable Graph

\`\`\`mermaid
flowchart LR
    classDef green fill:#1b5e20,color:#fff,stroke:#2e7d32
    classDef amber fill:#7a5c00,color:#fff,stroke:#b58900
    classDef red   fill:#7a1c1c,color:#fff,stroke:#c62828

    player-move-map["Player can move around the map"]:::amber
    2d-tile-map["2D top-down tile map"]:::red

    player-move-map --> 2d-tile-map
\`\`\`

## Build order
1. 2D top-down tile map  *(incomplete — open: rendering)*
2. Player can move around the map  *(blocked on above)*
```

Arrows point A → dependency. Use the node `id` as the Mermaid node key and the `title` as its label. Render the **global** graph by default; render a **rooted subgraph** (one node + its transitive dependencies) when the user asks to focus or the graph is large.

## Workflows

Run the status algorithm and re-render `_graph.md` after **every** mutation.

### Create
1. Read the deliverable statement. Extract every load-bearing aspect (nouns/capabilities the outcome depends on).
2. **Scan existing `Deliverables/`** for nodes that match an aspect by title/intent. For each match, **propose linking** before proposing a new node.
3. Present the proposed requirements. For each, ask the user to: **link** (existing or new deliverable), satisfy **inline**, mark **clear**, or **dismiss** (not a real requirement).
4. If the user wants new deliverables, recurse into Create for each (or leave them `[open]` for now if they say so).
5. Write the file with a stable `id`/slug. If every aspect is trivial, offer `clear_enough: true`.
6. Recompute + re-render.

### Modify
- **Structural change** (add/remove a requirement, change a link, mark clear, set `clear_enough`): apply, then silently recompute + re-render.
- **Prose change** (reword the statement/scope): re-extract requirements for **that node only** and re-ratify as in Create. If the change *loosens* a definition other nodes depend on, **flag** the affected dependents and ask whether to re-scan them — do **not** auto-cascade.

### Explore (read-only)
Show the node's statement, its requirements with states, its dependents (who links to it), and its computed local status. Offer to render its rooted subgraph. Change nothing, ask nothing.

### Delete
Remove the file, flip every `[linked] → <deleted id>` line in dependents back to `[open]`, then recompute + re-render.

## Rules

- You **propose**, the user **judges**. Never decide an aspect is "obvious" and drop it without asking; never mark something `[clear]` yourself.
- Create `Deliverables/` lazily on the first node.
- Cycles are always an error — never write a graph containing one.
- Recompute status from scratch every time; never trust or store a prior status value.
