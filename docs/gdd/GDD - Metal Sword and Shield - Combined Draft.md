# Metal Sword and Shield — Game Design Document (Combined Draft)

Last updated: 2025-08-20
Author: DermanDanisman (drafted with Copilot)

Purpose
- Single-player top-down action RPG design document combining prior notes and refined primary attribute progressions. This doc is intended to be clear and implementable for prototyping.

High-level concept
- Title: Metal Sword and Shield
- Genre: Top-Down Action RPG (single-player; future co-op possible)
- Core pillars: responsive real-time combat, attribute-driven builds, loot and itemization, tactical defensive mechanics (block/dodge), and modular prototyping.
- Inspirations: Diablo, Pillars of Eternity, Path of Exile, Tyranny

Platforms and scope
- Primary: PC. Consoles post-prototype.
- Priority: produce a vertical slice (one playable class fully implemented, 3 enemy types, 2 environments, basic loot and progression) as Milestone 1.

Glossary / Naming conventions
- Core primary attribute names: Strength, Dexterity, Intelligence, Endurance, Vigor.
  - Endurance: reduces incoming physical damage through armor scaling, increases physical resistance and crowd-control resistance.
  - Vigor: physical endurance (HP &amp; Stamina).
- Vital attributes: Health, Mana, Stamina
- Modifier math note: Attribute-based modifiers use a linear transform M = (A + Pre) * Coef + Post. Modifiers are applied in array order (top-to-bottom); each modifier operates on the running result.

1. Core systems (overview)
- Attributes &amp; derived stats pipeline
- Combat: basic attacks, skills, dodges, blocks, resources
- Items &amp; loot: weapons, armor, accessories, affixes
- Progression: XP → levels → attributes, class trees, equipment
- Quests &amp; exploration: main quest + side/hidden content
- UI: minimal HUD, hotbar, inventory, floating combat text
- Engine &amp; tech: Unreal Engine recommended with GAS if using UE

2. Player classes (initial roster)
- Warrior (melee tank)
  - Primary Attributes: Strength, Vigor
  - Role: front-line brawler with high survivability
  - Core mechanics: heavy attacks, shield block, War Cry buff
- Ranger (ranged DPS)
  - Primary Attributes: Dexterity, Strength
  - Role: mobile ranged damage dealer
  - Core mechanics: Rapid Shot, Dodge Roll, Arrow Rain
- Sorcerer (caster)
  - Primary Attributes: Intelligence, Vigor
  - Role: area &amp; burst magic damage; utility via crowd-control
  - Core mechanics: Fireball, Mana Shield, Arcane Blast

Class structure
- Each class: 1 basic attack, 3 active skills, 1 passive/talent tree starter node.
- Abilities scale from attributes using the derived-stat formulas in section 4.

3. Attributes
Core primary attributes (player &amp; enemies)
- Strength: physical damage, armor penetration, and influences some weapon-specific bonuses.
- Dexterity: attack/cast speed, cooldown reduction, mobility, and critical-hit mechanics.
- Intelligence: spell power, max mana, and mana regeneration.
- Endurance: physical resistance, base armor scaling and crowd-control resistance.
- Vigor: max health, stamina pool, health/stamina regen.

Vital attributes
- Health (HP): when 0 → death
- Mana: spent for spells/skills
- Stamina: spent for dodge/heavy attacks/blocks

4. Secondary stats &amp; sample formulas
- Use general transform S = (Coef × (A + Pre)) + Post where A is backing primary attribute. This keeps compatibility with attribute-based magnitude systems.

Sample starting formulas (tunable):
- Attack Power (physical) = (1.5 × (Strength + WeaponDamage)) + 0
- Spell Power = (1.5 × (Intelligence + SpellBase)) + 0
- Armor = 1.25 * (Endurance + 5)
- Armor Penetration (%) = 0.45 * (Strength + 3)
- Block Chance (%) = clamp( (Armor * 0.2), 0, 60 )
- Crit Chance (%) = clamp( 0.4 * (Dexterity + 2) + (ArmorPenetration * 0.1), 0, 95 )
- Crit Damage (%) = (1.15 * Dexterity) + (ArmorPenetration * 0.2) + 50
- Crit Resistance (%) = (0.5 * Armor)
- Evasion (%) = (0.3 * (Dexterity + Endurance)) + 2
- Max Health = (10 * Vigor) + 50
- Health Regen (HP/sec) = (0.5 * (Vigor + 1))
- Max Mana = (5 * Intelligence) + 25
- Mana Regen (MP/sec) = 1.0 * (Intelligence) + 3
- Max Stamina = (10 * Vigor) + 50
- Stamina Regen = (0.5 * (Vigor + 1))

Balancing note: coefficients are initial. Run DPS, survivability and encounter pacing tests and adjust.

5. Combat design
Controls &amp; loop
- Movement: click-to-move; hold-to-follow; camera zoom/edge scroll; shift-to-attack-in-place.
- Attacks: LMB basic; skills on hotkeys (Q/W/E/R or mouse buttons); defensive dodge (stamina) and block (stamina or passive reduction).
- Damage types: Physical, Magical, True
- Feedback: floating combat text, hit SFX, stagger, screenshake, death dissolve.

Abilities &amp; resources
- Physical abilities use Stamina; magical abilities use Mana; hybrid can use both.
- Cooldowns reduced by Dexterity modifiers.
- Critical strikes apply Crit Damage multiplier.

6. Enemy design &amp; AI
- Archetypes: Melee Warrior (tank), Ranged Archer (DPS), Sorcerer (caster)
- AI states: Idle, Investigate, Chase, Attack, Flee
- Positioning rules: melee close, ranged maintain distance, casters kite

7. Items, loot &amp; progression
- Item tiers: Common → Legendary
- Affix system: prefix/suffix model (stat bonuses, unique effects)
- Drops: zone-based drop tables; vendor buy/sell; crafting and enchanting planned

8. Skill trees &amp; progression
- Per-class trees with branching nodes for build depth (specs such as Tank/Berserker for Warrior)
- Passive nodes include stat increases and quality-of-life mechanics
- Respec: available with cost (gold or item)

9. Quests &amp; zones
- Main quest: collect pieces of a legendary weapon
- Example demo zones: Dark Forest (starter), Undercaves (mid), Fortress Ruins (boss)
- Side quests and exploration quests encourage replay

10. UX/UI &amp; accessibility
- HUD: Health/Mana/Stamina, hotbar, minimap optional
- Tooltips: attribute/derived stat explanations
- Accessibility: text scaling, colorblind friendly damage colors

11. Art &amp; audio
- Art direction: gritty semi-realistic top-down; consistent silhouette language for readability
- Audio: impactful hits, region ambiance, clear UI SFX

12. Technical &amp; prototyping recommendations
- Engine: Unreal Engine with GAS recommended
- Prototype scope: one class fully playable, 3 enemies, 2 zones, basic loot and inventory
- Data-driven: attributes, abilities and items should be external data (CSV/JSON/DataTables)

13. Balancing &amp; testing plan
- Instrumentation: log DPS, ability uptime, time-to-kill
- Small-number tests: validate with simple constants (Health 10, Vigor 9, etc.)
- Clamp derived stats during live gameplay; allow disabling for testing

14. Roadmap
- Milestone 0: Design &amp; repo setup
- Milestone 1: Core prototype (movement, 1 class, 3 abilities, 3 enemies, 1 zone)
- Milestone 2: Loot &amp; progression system
- Milestone 3: Polish &amp; internal testing

15. Primary attribute progression (level tables)
These tables provide base attribute values per level for the three starter classes. They are tuned for smooth scaling and clear class roles. Use as BaseAttribute templates in the engine and let items/talents apply modifiers.

Warrior (tank)
| Level | 1  | 5  | 10 | 15 | 20 | 25 |
|-------|----|----|----|----|----|----|
| Strength | 12 | 18 | 26 | 34 | 42 | 50 |
| Dexterity | 7 | 9 | 12 | 14 | 16 | 18 |
| Intelligence | 5 | 6 | 7 | 8 | 9 | 10 |
| Endurance | 10 | 13 | 16 | 20 | 24 | 28 |
| Vigor | 10 | 14 | 19 | 25 | 32 | 40 |

Ranger (ranged DPS)
| Level | 1  | 5  | 10 | 15 | 20 | 25 |
|-------|----|----|----|----|----|----|
| Strength | 8 | 10 | 13 | 16 | 20 | 24 |
| Dexterity | 10 | 15 | 22 | 30 | 38 | 46 |
| Intelligence | 7 | 9 | 11 | 14 | 17 | 20 |
| Endurance | 6 | 8 | 10 | 13 | 16 | 20 |
| Vigor | 7 | 9 | 12 | 15 | 18 | 22 |

Sorcerer (caster)
| Level | 1  | 5  | 10 | 15 | 20 | 25 |
|-------|----|----|----|----|----|----|
| Strength | 5 | 6 | 7 | 8 | 9 | 10 |
| Dexterity | 4 | 6 | 8 | 10 | 12 | 14 |
| Intelligence | 10 | 16 | 24 | 34 | 44 | 56 |
| Endurance | 7 | 9 | 11 | 13 | 16 | 20 |
| Vigor | 6 | 8 | 10 | 13 | 16 | 20 |

Appendix — next steps
- Produce per-class ability spreadsheets with resource costs, cooldowns and attribute scalars
- Convert these tables into DataTables (CSV) or GAS Attribute specs