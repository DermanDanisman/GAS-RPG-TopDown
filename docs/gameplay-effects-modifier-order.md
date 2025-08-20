# Gameplay Effects Modifier Order of Operations

Last updated: 2025-01-19

## Overview

Gameplay Effect modifiers are applied in a specific order (top-to-bottom in the modifiers array) to the target attribute. Understanding this order is crucial for creating predictable gameplay mechanics and avoiding unexpected behaviors.

- **Order matters**: Modifiers execute sequentially from top to bottom
- **Math operations**: Add, Multiply, and Divide operations compound based on execution order
- **Attribute-based magnitudes**: Values can be sourced from other attributes (Vigor, Strength, etc.)
- **Clamping interaction**: Final results may be clamped, but order affects intermediate calculations

## Worked Examples

### Example 1: Sequential Addition
```
Initial Health: 10
Modifier 1: +Vigor (9)
Modifier 2: +Strength (10) 
Modifier 3: +Resilience (12)

Calculation:
Health = 10 + 9 + 10 + 12 = 41
Final Result: 41
```

### Example 2: Add, Multiply, Add
```
Initial Health: 10
Modifier 1: +Vigor (9)
Modifier 2: *Strength (10)
Modifier 3: +Resilience (12)

Calculation:
Step 1: Health = 10 + 9 = 19
Step 2: Health = 19 * 10 = 190
Step 3: Health = 190 + 12 = 202
Final Result: 202
```

### Example 3: Add, Multiply, Divide
```
Initial Health: 10
Modifier 1: +Vigor (9)
Modifier 2: *Strength (10)
Modifier 3: /Resilience (12)

Calculation:
Step 1: Health = 10 + 9 = 19
Step 2: Health = 19 * 10 = 190
Step 3: Health = 190 / 12 ≈ 15.83
Final Result: ≈15.83
```

### Example 4: Chained with Max Attribute
```
Previous chain result: ≈15.83
Modifier 4: +MaxHealth (100)

Calculation:
Health = 15.83 + 100 ≈ 115.83
Final Result: ≈115.83
```

## Clamping Considerations

- **Test environment**: Examples above assume clamping is disabled for verification
- **Production usage**: Always keep attribute clamping enabled in real games
- **Callback order**: PreAttributeChange and PostGameplayEffectExecute handle clamping
- **Overflow prevention**: Proper clamping prevents "invisible buffer" issues
- **Verification tool**: Use `showdebug abilitysystem` to verify calculations in-game

## Tips and Common Pitfalls

- **Exact order matters**: Rearranging modifiers in the array changes results significantly
- **Test with simple numbers**: Use round numbers during development for easier debugging
- **Override for initialization**: Prefer Override operation for setting initial attribute values
- **Server/client replication**: Be aware of timing differences when testing networked gameplay
- **Magnitude sources**: Verify that attribute-based magnitudes reference the correct source attributes
- **Floating point precision**: Division operations may introduce small precision errors

## Troubleshooting

- **Unexpected results**: Double-check modifier order in the Gameplay Effect asset
- **Values not applying**: Verify attribute references and magnitude calculation types
- **Replication issues**: Ensure effects are applied on the authoritative side (usually server)
- **Clamping interference**: Temporarily disable clamping to isolate calculation issues
- **Debug tools**: Use Gameplay Debugger and console commands for real-time inspection

## See Also

- [Gameplay Effects](gameplay-effects.md) - General Gameplay Effects documentation
- [Attribute Clamping](attributes/attribute-clamping.md) - Comprehensive clamping guide