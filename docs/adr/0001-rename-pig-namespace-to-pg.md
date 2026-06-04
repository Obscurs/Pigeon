# ADR 0001 — Rename engine namespace from `pig` to `pg`

**Status:** Accepted  
**Date:** 2026-06-04

## Context

The PigeonLib engine used `pig` as its primary C++ namespace. While derivable from "Pigeon", the three-letter abbreviation `pig` is an unintended word with negative connotations, and `pg` is both shorter and a cleaner two-letter prefix consistent with common engine conventions.

## Decision

Rename the `pig` namespace to `pg` (and `pg::ui` to `pg::ui`) across all files in the repository. Folder names, project names (`PigeonLib`, `Pigeon`), and the `sbx` sandbox namespace are unchanged.

## Trade-offs

- **`pig` (old):** Derives directly from "Pigeon"; self-documenting in the source.
- **`pg` (new):** Shorter, avoids the unintended word, consistent two-letter prefix pattern.

## Consequences

All qualified names (`pg::System`, `pg::World`, etc.) become `pg::System`, `pg::World`, etc. Documentation and code samples must be updated accordingly.
