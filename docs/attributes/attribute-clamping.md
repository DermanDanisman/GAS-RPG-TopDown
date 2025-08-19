# Attribute Clamping in GAS

Last updated: 2025-08-19

## Overview

Attribute clamping is critical for maintaining game balance and preventing exploits in Gameplay Ability System (GAS). This document explains the three-tier clamping strategy used in this project and how to avoid the common "invisible buffer" bug.

## The Three Clamping Callbacks

### 1. PreAttributeChange()

**When it fires:**
- Before any CurrentValue modification from any source
- Duration/Infinite GameplayEffects applying/removing modifiers
- Direct attribute sets (SetHealth, etc.)
- Aggregator recalculations

**Purpose:**
- Clamp the visible CurrentValue to valid ranges [0, MaxX]
- React to Max attribute changes (when MaxHealth decreases, clamp Health accordingly)

**Key Limitation:**
- Clamping `NewValue` here does NOT persist to underlying aggregator inputs
- Duration/Infinite effect modifiers remain unchanged in the aggregator
- This can lead to "invisible buffer" scenarios

**Example:**
```cpp
void UCoreAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
    }
}
```

### 2. PreAttributeBaseChange()

**When it fires:**
- Only for BaseValue changes from Instant/Periodic GameplayEffects
- Does NOT fire for Duration/Infinite effects
- Does NOT fire for direct attribute sets

**Purpose:**
- Clamp BaseValue to [0, MaxX] preventing permanent overflow
- Prevent "invisible buffer" where BaseValue > MaxValue
- Critical for Instant healing potions and Periodic effects

**Example:**
```cpp
void UCoreAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
    }
}
```

### 3. PostGameplayEffectExecute()

**When it fires:**
- After Instant/Periodic GameplayEffects complete execution
- Does NOT fire for Duration/Infinite effects
- Server-authoritative in networked games

**Purpose:**
- Final authoritative clamping using SetX() methods
- React to attribute changes (adjust current when max changes)
- Handle damage/healing resolution, death checks

**Example:**
```cpp
void UCoreAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    // Final clamping when Max attributes change
    if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
    }
}
```

## GameplayEffect Types and Clamping

### Instant Effects
- **Duration Policy:** Instant
- **Behavior:** Executes once, modifies BaseValue permanently
- **Callbacks:** PreAttributeBaseChange → PreAttributeChange → PostGameplayEffectExecute
- **Use Cases:** Health potions, permanent stat boosts
- **Clamping:** All three callbacks fire for maximum protection

### Duration Effects (HasDuration)
- **Duration Policy:** HasDuration
- **Behavior:** Temporary CurrentValue modifier, expires automatically
- **Callbacks:** Only PreAttributeChange (when applied/removed)
- **Use Cases:** Temporary buffs, time-limited bonuses
- **Clamping:** Limited to PreAttributeChange only

### Periodic Effects
- **Duration Policy:** HasDuration or Infinite with Period > 0
- **Behavior:** Executes repeatedly at intervals (each tick is like an Instant effect)
- **Callbacks:** All three callbacks fire for each tick
- **Use Cases:** Regeneration over time, damage over time
- **Clamping:** Full protection through all callbacks

### Infinite Effects
- **Duration Policy:** Infinite
- **Behavior:** Temporary CurrentValue modifier until explicitly removed
- **Callbacks:** Only PreAttributeChange (when applied/removed)
- **Use Cases:** Auras, persistent buffs, equipment bonuses
- **Clamping:** Limited to PreAttributeChange only

## BaseValue vs CurrentValue

### BaseValue
- The permanent, underlying value of an attribute
- Modified only by Instant/Periodic GameplayEffects
- Persists through game sessions (if saved)
- Forms the foundation for CurrentValue calculations

### CurrentValue
- The final computed value after all modifiers
- Calculated as: BaseValue + Sum(All Active Modifiers)
- What players see and what gameplay systems use
- Temporary modifiers from Duration/Infinite effects affect this

### The Calculation
```
CurrentValue = BaseValue + Additive Modifiers + (BaseValue * Multiplicative Modifiers)
```

## The "Fire Area + Health Crystals" Bug

### The Scenario
1. Player has Health=50, MaxHealth=100
2. Player steps into a fire area (Infinite GE with -5 Health/second)
3. Player picks up health crystals (+50 Health each, Duration GE for 10 seconds)
4. Player picks up multiple crystals, expecting NewValue to be clamped to MaxHealth

### What Goes Wrong (Without Proper Clamping)
```
Initial: Health=50, MaxHealth=100
Fire area: Health = 50 + (-5/sec modifier) = visible 45
Crystal 1: Health = 50 + (-5/sec) + 50 = visible 95
Crystal 2: Health = 50 + (-5/sec) + 50 + 50 = clamped to 100 (visible)
```

**The Bug:** In PreAttributeChange, NewValue shows as 145 but gets clamped to 100 for display. However, the +50 modifiers remain in the aggregator!

When fire area is removed:
```
After leaving fire: Health = 50 + 50 + 50 = 150 (exceeds MaxHealth!)
```

### Why It Happens
- PreAttributeChange clamping only affects the visible value
- The underlying aggregator keeps all modifiers intact
- When one modifier is removed, hidden overflow resurfaces

### The Solution: Three-Tier Clamping
Our implementation prevents this through:

1. **PreAttributeChange:** Immediate visible clamping
2. **PreAttributeBaseChange:** Prevents BaseValue overflow from Instant/Periodic effects  
3. **PostGameplayEffectExecute:** Final authoritative adjustment when Max attributes change

## Implementation in This Project

### CoreAttributeSet Strategy
```cpp
// Tier 1: Visible value clamping (all sources)
void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
    }
}

// Tier 2: BaseValue clamping (Instant/Periodic only)
void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
    }
}

// Tier 3: Final authoritative adjustment
void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
    }
}
```

### Why This Works
- PreAttributeChange prevents visible overflow from any source
- PreAttributeBaseChange prevents permanent BaseValue overflow  
- PostGameplayEffectExecute provides final safety net when Max values change
- The combination eliminates all known overflow scenarios

## Troubleshooting Common Issues

### Issue: Attributes exceed maximum values
**Symptoms:** Health shows 150/100, Mana at 75/50
**Cause:** Missing or incomplete clamping implementation
**Solution:** Implement all three callback tiers for affected attributes

### Issue: Attributes don't update when Max changes
**Symptoms:** MaxHealth increases but Health stays at old maximum
**Cause:** Missing PostGameplayEffectExecute clamping
**Solution:** Add Max attribute change detection in PostGameplayEffectExecute

### Issue: Duration effects create permanent overflow
**Symptoms:** Temporary buffs leave permanent attribute increases
**Cause:** Duration/Infinite effects only use PreAttributeChange
**Solution:** This is expected behavior; use Instant effects for permanent changes

### Issue: Periodic effects don't respect limits
**Symptoms:** Health crystals with periodic ticking exceed MaxHealth
**Cause:** Each periodic tick is treated as Instant, but clamping may be incomplete
**Solution:** Ensure all three callbacks handle the affected attribute

## Replication Considerations

### Server Authority
- PostGameplayEffectExecute typically runs only on the server
- Attribute changes replicate to clients via RepNotify functions
- Clients receive final clamped values through normal replication

### Client Prediction
- Clients may predict attribute changes locally
- Server reconciliation ensures authoritative values
- RepNotify functions help synchronize client UI

### Network Optimization
- Avoid excessive attribute modifications that trigger constant replication
- Consider batching multiple attribute changes when possible
- Use appropriate replication conditions (COND_None vs COND_SkipOwner)

## Best Practices

### Do:
- Implement all three callback tiers for critical attributes
- Use PostGameplayEffectExecute for final authoritative adjustments
- Test with Duration/Infinite effects to verify no hidden overflow
- Guard debug output with shipping build checks (#if !UE_BUILD_SHIPPING)

### Don't:
- Rely solely on PreAttributeChange for complete clamping
- Modify attributes directly in PreAttributeChange/PreAttributeBaseChange
- Ignore Max attribute changes in PostGameplayEffectExecute
- Use GEngine->AddOnScreenDebugMessage in shipping builds

### Testing Checklist:
- [ ] Instant effects respect attribute limits
- [ ] Duration effects don't create hidden overflow  
- [ ] Max attribute changes adjust current values appropriately
- [ ] Multiple overlapping effects clamp correctly
- [ ] Periodic effects respect limits on each tick
- [ ] Network replication maintains clamped values

## See Also

- [GAS Attribute Callbacks Cheatsheet](../cheatsheets/gas-attribute-callbacks.md)
- [Attributes & Accessors](../attributes-and-accessors.md)
- [Gameplay Effects](../gameplay-effects.md)
- [Replication & Multiplayer](../replication-and-multiplayer.md)