# Contributing

Last updated: 2025-08-17

Thanks for helping improve the project and its documentation. This repo is in active learning mode; please treat docs as living and iterative.

## Docs changes

- Keep documents short, scoped, and practical.
- When updating code behavior, update the nearest doc section in the same PR.
- Add "Last updated: YYYY-MM-DD" at the top of edited docs.
- Prefer examples grounded in the current code snapshot.

## Code changes

- Follow Unreal C++ style used in the repo.
- Keep GAS responsibilities separated (ASC, AttributeSet, Effects, Tags, UI Controller).
- Multiplayer: favor server authority; keep client changes predictable.

## Opening PRs

- Title: concise and action-oriented (e.g., "Docs: clarify periodic effects vs current/base").
- Include before/after or a brief rationale.
- Link to relevant course-note section when applicable.

## Issues

- Use the "Docs Update" template for documentation fixes/requests.
- Use labels: docs, gas, ui, effects, multiplayer, tags, plugin.