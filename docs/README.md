# Documentation Index

Last updated: 2024-12-19

> This documentation is a living document. If anything is outdated or unclear, please open a "Docs Update" issue or a small PR.

## Getting Started
- Setup: getting-started/setup.md

## Architecture
- Overview: architecture/overview.md
- Replication & Multiplayer: architecture/replication-and-multiplayer.md

## Gameplay Ability System (GAS)
- Gameplay Effects: gas/gameplay-effects.md
- Gameplay Tags: gas/gameplay-tags.md

### How-to Guides (Step-by-step)
- End-to-end Setup (Input tags → ASC → Ability activation): gas/howto/end-to-end-setup.md
- Runtime Remapping of Ability Input Tags: gas/howto/runtime-remapping.md  
- Troubleshooting Input Tags → Ability Activation: gas/howto/troubleshooting.md

### Abilities
- Ability Input Tags and Activation Flow (ASC-centric): gas/abilities/ability-input-tags-and-activation.md
- Overview: gas/abilities/overview.md
- Base Ability and Startup Grant: gas/abilities/base-ability-and-startup-grant.md
- Ability Tags and Policies: gas/abilities/ability-tags-and-policies.md
- Projectiles in GAS Abilities: gas/abilities/projectiles.md

### Attributes
- Attribute Clamping: gas/attributes/attribute-clamping.md
- Derived Attributes: gas/attributes/derived-attributes.md
- Custom Calculations: gas/attributes/custom-calculations.md
- Derived Attributes Table (CSV): gas/attributes/derived-attributes-table.csv

### Cheatsheets
- GAS Attribute Callbacks: gas/cheatsheets/gas-attribute-callbacks.md

### Input
- Enhanced Input → Ability Mapping (TD Input Config): gas/input/enhanced-input-to-abilities.md
- Binding callbacks with UTDEnhancedInputComponent (Ability Inputs): gas/input/td-enhanced-input-component.md

## UI
- UI & Widget Controller: ui/ui-widget-controller.md

### Attribute Menu
- Attribute Menu Container: ui/attribute-menu/attribute-menu.md
- Attribute Menu Button: ui/attribute-menu/attribute-menu-button.md
- Attribute Menu Widget Controller: ui/attribute-menu/attribute-menu-controller.md
- Attribute Menu Widget Controller Setup: ui/attribute-menu/attribute-menu-widget-controller-setup.md
- Attribute Menu Broadcasting: ui/attribute-menu/AttributeMenu_Broadcasting.md
- Framed Value: ui/attribute-menu/framed-value.md
- Text Value Row: ui/attribute-menu/text-value-row.md
- Text Value Button Row: ui/attribute-menu/text-value-button-row.md

### Button
- WP_Button Widget: ui/button/wp-button.md

## Data
- Attribute Info: data/attribute-info.md
- Secondary Attributes Tags CSV: data/Content_GASCore_GameplayTags_DT_SecondaryAttributesTags_Version3.csv

## Systems
- Gameplay Tags Centralization: systems/gameplay-tags-centralization.md
- TD Gameplay Tags: systems/td-gameplay-tags.md
- Attributes Gameplay Tags: systems/attributes-gameplay-tags.md

## Plugin
- Plugin API (Proposed): plugin/api.md

## Guides
- Course Notes mapping: guides/course-notes.md
- Gameplay Abilities — Concepts and Practice (Deep Dive): guides/GameplayAbilities_Concepts_And_Practice.md
- BaseGameplayAbility Pattern and Startup Grant (Deep Dive): guides/BaseGameplayAbility_Pattern_StartupGrant.md
- Instructor's Implementation Deep Dive: guides/Instructor_Implementation_DeepDive.md

## Game Design Documents
- gdd/attribute_formulas.cpp

## Contributing
- Contributing Guide: ../CONTRIBUTING.md