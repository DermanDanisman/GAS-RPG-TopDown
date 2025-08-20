# Documentation Index

> This documentation is a living document. It will evolve as we progress through the course and as the codebase changes. If something is out of date or unclear, please open a "Docs Update" issue or a PR.

Last updated: 2025-08-19

## Index

- Project overview: ../README.md
- Setup: setup.md
- Architecture: architecture.md
- Attributes & Accessors: attributes-and-accessors.md
- UI & Widget Controller: ui-widget-controller.md
- Gameplay Effects: gameplay-effects.md
- Replication & Multiplayer: replication-and-multiplayer.md
- Gameplay Tags: gameplay-tags.md
- Debugging & Tools: debugging-and-tools.md
- Course Notes mapping: course-notes.md
- Plugin design (WIP):
  - plugin/overview.md
  - plugin/api.md

## Attribute System Deep Dives

- **[Attribute Clamping Guide](attributes/attribute-clamping.md)** - Comprehensive guide to preventing attribute overflow bugs
- **[GAS Callback Cheatsheet](cheatsheets/gas-attribute-callbacks.md)** - Quick reference for AttributeSet callback usage

## Gameplay Effects Deep Dives

- **[Modifier Order of Operations](gameplay-effects-modifier-order.md)** - How modifiers are applied sequentially and why order matters

## How to propose changes

- Prefer small, incremental PRs for doc changes.
- Alternatively, use the "Docs Update" issue template (see .github/ISSUE_TEMPLATE) describing:
  - What's outdated or missing
  - Where it's documented
  - Relevant code/commits or screenshots