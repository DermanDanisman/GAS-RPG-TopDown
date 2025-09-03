# Attributes and Accessors

Last updated: 2024-12-19

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
ATTRIBUTE_ACCESSORS(UTDAttributeSet, Health);
ATTRIBUTE_ACCESSORS(UTDAttributeSet, MaxHealth);
ATTRIBUTE_ACCESSORS(UTDAttributeSet, Mana);
ATTRIBUTE_ACCESSORS(UTDAttributeSet, MaxMana);
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