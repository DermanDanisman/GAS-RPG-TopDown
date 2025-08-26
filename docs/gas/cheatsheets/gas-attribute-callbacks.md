# GAS Attribute Callbacks Cheatsheet

Last updated: 2025-08-19

Quick reference for when each AttributeSet callback fires and what to do in each one.

## Callback Overview Table

| Callback | When It Fires | GameplayEffect Types | Purpose | Safe Operations |
|----------|---------------|---------------------|---------|-----------------|
| `PreAttributeChange` | Before CurrentValue changes | All sources | Clamp visible values | Modify `NewValue` parameter |
| `PreAttributeBaseChange` | Before BaseValue changes | Instant, Periodic | Prevent BaseValue overflow | Modify `NewValue` parameter |
| `PostGameplayEffectExecute` | After GE execution | Instant, Periodic | Final adjustments & reactions | Use `SetX()` methods |

## PreAttributeChange()

### ⏰ When It Fires
```
✅ Duration/Infinite GE apply/remove
✅ Direct attribute sets (SetHealth)
✅ Aggregator recalculations
✅ Any CurrentValue modification
❌ BaseValue-only changes
```

### 🎯 What To Do
```cpp
void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // Clamp visible CurrentValue to valid ranges
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
    }
}
```

### ⚠️ Limitations
- Clamping here doesn't affect underlying aggregator modifiers
- Duration/Infinite effect values remain in aggregator even if clamped
- Can lead to "invisible buffer" scenarios

---

## PreAttributeBaseChange()

### ⏰ When It Fires
```
✅ Instant GameplayEffects
✅ Periodic GameplayEffect ticks
❌ Duration GameplayEffects
❌ Infinite GameplayEffects
❌ Direct attribute sets
```

### 🎯 What To Do
```cpp
void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    // Prevent permanent BaseValue overflow
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
    }
}
```

### ✅ Key Benefits
- Prevents "invisible buffer" where BaseValue > MaxValue
- Critical for permanent modifications (potions, level-ups)
- Stops periodic effects from creating permanent overflow

---

## PostGameplayEffectExecute()

### ⏰ When It Fires
```
✅ After Instant GE execution
✅ After Periodic GE tick execution
❌ Duration GE apply/remove
❌ Infinite GE apply/remove
❌ Direct attribute sets
```

### 🎯 What To Do
```cpp
void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    // Final authoritative clamping when Max attributes change
    if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
    }
    
    // Handle damage/heal meta-attributes
    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        const float LocalDamage = GetDamage();
        SetDamage(0.f); // Clear meta-attribute
        SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
    }
}
```

### 🔒 Network Authority
- Typically runs only on server in multiplayer
- Changes replicate to clients via RepNotify
- Server-authoritative for final attribute values

---

## GameplayEffect Type Reference

### Instant Effects
```
Duration Policy: Instant
BaseValue: ✅ Modified permanently
CurrentValue: ✅ Recalculated from new BaseValue
Callbacks: PreAttributeBaseChange → PreAttributeChange → PostGameplayEffectExecute
Use Cases: Health potions, permanent stat increases
```

### Duration Effects
```
Duration Policy: HasDuration
BaseValue: ❌ Not modified
CurrentValue: ✅ Temporarily modified via aggregator
Callbacks: PreAttributeChange (on apply/remove)
Use Cases: Temporary buffs, timed bonuses
```

### Periodic Effects
```
Duration Policy: HasDuration or Infinite + Period > 0
BaseValue: ✅ Modified on each tick (like multiple Instant effects)
CurrentValue: ✅ Recalculated after each tick
Callbacks: All three callbacks fire for each tick
Use Cases: Health regen, damage over time
```

### Infinite Effects
```
Duration Policy: Infinite
BaseValue: ❌ Not modified
CurrentValue: ✅ Modified until explicitly removed
Callbacks: PreAttributeChange (on apply/remove)
Use Cases: Equipment bonuses, auras, persistent buffs
```

---

## Common Patterns

### Basic Attribute Clamping
```cpp
// In all three callbacks for complete protection
if (Attribute == GetHealthAttribute())
{
    NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
}
```

### Max Attribute Change Handling
```cpp
// Only in PostGameplayEffectExecute
if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
{
    SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
}
```

### Meta-Attribute Processing
```cpp
// Only in PostGameplayEffectExecute
if (Data.EvaluatedData.Attribute == GetDamageAttribute())
{
    const float LocalDamage = GetDamage();
    SetDamage(0.f); // Clear the meta-attribute
    SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
}
```

### Death Check
```cpp
// Only in PostGameplayEffectExecute
if (GetHealth() <= 0.f && !bCharacterDead)
{
    bCharacterDead = true;
    // Trigger death logic, gameplay cues, etc.
}
```

---

## Troubleshooting Quick Guide

| Problem | Likely Cause | Solution |
|---------|-------------|----------|
| Attributes exceed max | Missing PreAttributeChange | Add clamping in PreAttributeChange |
| Hidden overflow after temporary effects | Missing PreAttributeBaseChange | Add clamping in PreAttributeBaseChange |
| Current doesn't adjust when max changes | Missing PostGameplayEffectExecute | Add max change handling in PostGameplayEffectExecute |
| Periodic effects overflow | Missing any callback | Implement all three callbacks |
| Network desync | Server/client logic differences | Ensure callbacks run consistently |

---

## BaseValue vs CurrentValue Quick Reference

### BaseValue
- **Modified by:** Instant, Periodic GameplayEffects
- **Persists:** Permanently (unless overwritten)
- **Callbacks:** PreAttributeBaseChange, PostGameplayEffectExecute
- **Network:** Replicates normally

### CurrentValue
- **Modified by:** All sources (BaseValue + Modifiers)
- **Calculation:** `BaseValue + Sum(Additive) + BaseValue * Sum(Multiplicative)`
- **Callbacks:** PreAttributeChange (always), PostGameplayEffectExecute (after calculation)
- **Network:** Computed value, replicates after calculation

---

## Safe Operations by Callback

### ✅ PreAttributeChange
```cpp
// Safe: Modify the NewValue parameter
NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);

// Safe: Read current attribute values
float CurrentHealth = GetHealth();
float MaxHealth = GetMaxHealth();
```

### ✅ PreAttributeBaseChange  
```cpp
// Safe: Modify the NewValue parameter
NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);

// Safe: Read current attribute values (const function)
float CurrentHealth = GetHealth();
```

### ✅ PostGameplayEffectExecute
```cpp
// Safe: Use SetX() methods to modify attributes
SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

// Safe: Clear meta-attributes
SetDamage(0.f);

// Safe: Trigger gameplay reactions
if (GetHealth() <= 0.f) { TriggerDeath(); }
```

### ❌ Never Do
```cpp
// Don't modify attributes directly in Pre* callbacks
SetHealth(NewValue); // ❌ Can cause infinite recursion

// Don't use GEngine debug messages in shipping
#if WITH_EDITOR // ✅ Guard debug output
    GEngine->AddOnScreenDebugMessage(...);
#endif
```

---

## See Also

- [Attribute Clamping Guide](../attributes/attribute-clamping.md)
- [Attributes & Accessors](../../attributes-and-accessors.md)
- [Gameplay Effects](../gameplay-effects.md)