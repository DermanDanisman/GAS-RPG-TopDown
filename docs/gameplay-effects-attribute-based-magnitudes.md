# Attribute-Based Magnitudes and Modifier Order of Operations

Last updated: 2025-08-20

Purpose
- Demystify Attribute-Based Magnitude in Gameplay Effects (GEs) and how modifier order changes outcomes.
- Explain Coefficient, Pre Multiply Additive, and Post Multiply Additive with clear formulas and examples.

Big picture
- Attribute-Based Magnitude lets one attribute change another using a simple linear transform and a chosen source (Source or Target).
- Multiple modifiers on the same attribute execute in array order (top-to-bottom), each operating on the running result.

Visual mental model
```
Initial Value ──(Modifier[0])──> R1 ──(Modifier[1])──> R2 ──(Modifier[2])──> R3 ── ... ──> Final
```

Attribute-Based Magnitude components
- Backing Attribute: the attribute whose value becomes the input to the magnitude calculation.
- Attribute Source:
  - Source: captured from the effect instigator/causer.
  - Target: captured from the actor receiving the effect.
- Snapshot (basic idea): when to capture the backing attribute's value.
  - Off (default here): capture at application time.
  - On: capture at spec creation time (useful for buffs/debuffs that should not change while active).

Magnitude formula (attribute-based)
- For a single modifier backed by attribute A with parameters:
  - Coefficient (Coef)
  - Pre Multiply Additive (Pre)
  - Post Multiply Additive (Post)
- The computed magnitude M is:
```
M = ( A + Pre ) * Coef + Post
```
- Intuition:
  - Pre shifts the input before scaling
  - Coef scales the shifted input
  - Post shifts the result after scaling

Modifier operations (what M does to the running result R)
- Add:        R = R + M
- Multiply:   R = R * M
- Divide:     R = R / M

Order of operations (array order matters)
- Modifiers are applied top-to-bottom. Each step uses the previous step's result.
- Reordering the array changes the result, especially with Multiply/Divide in the mix.

Worked examples (numbers from the course transcripts)
Assumptions for the first four examples:
- Health starts at 10 (for easy math)
- Target attributes: Vigor=9, Strength=10, Resilience=12, MaxHealth=100
- All magnitudes use Attribute-Based with no Pre/Coef/Post (i.e., effectively M=A)

1) Add + Add + Add
- Chain: +Vigor, +Strength, +Resilience
- Math:
```
10 + 9 = 19
19 + 10 = 29
29 + 12 = 41
```
- Result: 41

2) Add, then Multiply, then Add
- Chain: +Vigor, *Strength, +Resilience
- Math (clamping disabled for demo):
```
10 + 9 = 19
19 * 10 = 190
190 + 12 = 202
```
- Result: 202

3) Add, then Multiply, then Divide
- Chain: +Vigor, *Strength, /Resilience
- Math:
```
10 + 9 = 19
19 * 10 = 190
190 / 12 ≈ 15.83
```
- Result: ≈ 15.83

4) Previous chain, then Add MaxHealth
- Chain: +Vigor, *Strength, /Resilience, +MaxHealth
- Math:
```
(190 / 12) ≈ 15.83
15.83 + 100 ≈ 115.83
```
- Result: ≈ 115.83

Coefficient, Pre, Post explained with a combined example
- Now enable the linear transform M = (A + Pre) * Coef + Post for each modifier.
- Same starting values: Health=10; Target attributes Vigor=9, Strength=10, Resilience=12
- Modifiers targeting Health (in this order):
  1) Add with Vigor, Coef=0.1, Pre=3, Post=1
  2) Multiply with Strength, Coef=0.5, Pre=6, Post=11
  3) Divide with Resilience, Coef=2.1, Pre=0.5, Post=5.7

Step-by-step magnitudes
```
M1 (Vigor):     (9 + 3) * 0.1 + 1  = 12 * 0.1 + 1  = 1.2 + 1   = 2.2
M2 (Strength):  (10 + 6) * 0.5 + 11 = 16 * 0.5 + 11 = 8 + 11    = 19
M3 (Resilience):(12 + 0.5) * 2.1 + 5.7 = 12.5*2.1 + 5.7 = 26.25 + 5.7 = 31.95
```

Apply in order (visual and math)
```
Health = 10
  ├─ + M1 (2.2)            => 12.2
  ├─ * M2 (19)             => 231.8
  └─ / M3 (31.95)          => 231.8 / 31.95 ≈ 7.26
```
- Final Result: ≈ 7.26
- Teachable point: Coef/Pre/Post change the EFFECTIVE magnitudes that your Add/Multiply/Divide operations use. The order still applies exactly the same way.

Clamping and testing guidance
- To verify raw math (e.g., reaching 202), temporarily disable clamping in:
  - PreAttributeChange (for Current clamping)
  - PreAttributeBaseChange (for Base clamping of Instant/Periodic effects)
  - PostGameplayEffectExecute (authoritative clamp)
- In actual gameplay, keep clamping enabled to avoid overflow and invisible buffer problems.
- Use `showdebug abilitysystem` in PIE to observe attribute changes in real time.

Practical tips and pitfalls
- Ordering: The Modifiers array order is the order of application—rearrange to change outcomes.
- Source vs Target: Be explicit about where the backing attribute comes from.
- Snapshot: Decide whether to capture at spec creation or at application; this matters for buffs/debuffs that should not drift over time.
- Initialization GEs: Prefer Override modifiers for initial/default values.
- Replication & testing: Apply on the server and let attributes replicate to clients when validating math.
- Sanity checks: Test with simple numbers first (10, 9, 12, 100) before tuning game values.

See also
- Gameplay Effects overview: gameplay-effects.md
- Attribute clamping: attributes/attribute-clamping.md