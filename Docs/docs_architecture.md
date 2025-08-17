# Architecture Overview

This project follows a clear separation of concerns:

- Model (GAS): ASC, AttributeSet, GameplayEffects, GameplayTags
- Controller (Widget Controller): pulls from Model and broadcasts to View; also receives user inputs from widgets and applies to Model
- View (Widgets): display-only, bind to controller events

Key Classes/Concepts:

- Ability System Component (ASC)
  - Grants and activates abilities
  - Applies and replicates effects
  - Owns tags
  - Useful delegate: OnGameplayEffectAppliedDelegateToSelf

- Attribute Set
  - Stores GameplayAttributeData (Base/Current)
  - Boilderplate accessors macros
  - Use Effects for changes (prefer prediction & rollback)

- Widget Controller
  - Holds references to PlayerController, PlayerState, ASC, AttributeSet
  - Broadcast initial values (health/mana)
  - Subscribes to ASC/AttributeSet delegates and converts to UI signals

- Widgets
  - Call `SetWidgetController()`
  - Implement `WidgetControllerSet` (BP event) to bind to delegates
  - Use safe percentage calculations (Health / MaxHealth with safe divide)

- Effect Actor Pattern
  - Actor with overlap volume & data for what Effect to apply
  - Applies Instant/Duration/Infinite Effects to overlapping target ASC
  - Policies for Apply/Remove (see gameplay-effects.md)
```

````markdown name=docs/attributes-and-accessors.md
# Attributes and Accessors

## Defining Attributes

- Attributes are `FGameplayAttributeData` fields in your AttributeSet
- Example:
```cpp
UPROPERTY(BlueprintReadOnly, Category="Attributes")
FGameplayAttributeData Health;
```

- Base vs Current:
  - Instant effects modify Base (permanent)
  - Duration/Infinite effects modify Current (temporary, undone on removal)
  - Periodic changes are treated like Instant (permanent base changes each tick)

## Accessor Macros

Add to AttributeSet header:
```cpp
// Boilerplate to generate getters/setters
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
```

Use per attribute:
```cpp
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);
```

## Initialization

Use `InitXxx()` in the constructor (not `SetXxx()`):
```cpp
InitHealth(100.f);
InitMaxHealth(100.f);
InitMana(50.f);
InitMaxMana(50.f);
```

## Debugging

Console:
```
showdebug abilitysystem
```
View Owner, Avatar, owned tags, and current attribute values.