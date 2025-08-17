# Debugging and Tools

## Console

- `showdebug abilitysystem`
  - Owner/Avatar
  - Attributes
  - Tags
  - Ability info (when applicable)

## Logs/Screen

- `GEngine->AddOnScreenDebugMessage` for quick visual checks
- UE_LOG for persistent logging

## Delegates and Lambdas

- Prefer lambdas for simple delegate bindings:
```cpp
ASC->OnGameplayEffectAppliedDelegateToSelf.AddLambda(
  [this](UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
  {
    // handle
  }
);
```

## Blueprint vs C++

- Use BP for data-driven graphing (effects/overlaps)
- Use C++ for replication, policies, and delegate wiring