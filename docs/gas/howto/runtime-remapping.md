# Runtime remapping of ability input tags

Last updated: 2025-09-03

Goal: Change which input activates an ability at runtime without modifying the ability class.

Why it works: Input tags live in the FGameplayAbilitySpec's dynamic source tags, designed for runtime add/remove.

## Step-by-step (C++)
```cpp
// Find your ASC (cached) and loop specs to find the target ability
UGASCoreAbilitySystemComponent* ASC = /* Get your ASC */;
const FGameplayTag From = FTDGameplayTags::Get().InputTag_LMB;
const FGameplayTag To   = FTDGameplayTags::Get().InputTag_RMB;

for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
{
    FGameplayTagContainer& Dynamic = Spec.GetDynamicSpecSourceTags();
    if (Dynamic.HasTagExact(From))
    {
        Dynamic.RemoveTag(From);
        Dynamic.AddTag(To);
        // Optional: If ability is currently active and cares about input state,
        // you may want to signal release/press cycle:
        ASC->AbilitySpecInputReleased(Spec);
        ASC->AbilitySpecInputPressed(Spec);
    }
}
```

## Alternative: By ability class
If you want to remap all instances of a specific ability class:

```cpp
void UTDAbilitySystemComponent::RemapAbilityInputTag(TSubclassOf<UGameplayAbility> AbilityClass, const FGameplayTag& FromTag, const FGameplayTag& ToTag)
{
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
    {
        if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
        {
            FGameplayTagContainer& Dynamic = Spec.GetDynamicSpecSourceTags();
            if (Dynamic.HasTagExact(FromTag))
            {
                Dynamic.RemoveTag(FromTag);
                Dynamic.AddTag(ToTag);
                
                // Signal input state change if needed
                if (Spec.IsActive())
                {
                    AbilitySpecInputReleased(Spec);
                    AbilitySpecInputPressed(Spec);
                }
            }
        }
    }
}
```

## Tips
- Persist changes if needed (save system) since dynamic tags reset on new Spec creation
- Notify UI to refresh bindings after remap
- Avoid duplicate tags (RemoveTag before AddTag)
- Consider edge cases like multiple abilities mapped to the same input

## BP-friendly approach
- Expose a helper on your ASC to RemapInputTag(SpecHandle, From, To) or by AbilityClass
- Create a Blueprint function library for common remapping operations

## Example use cases
- Player customizes keybindings
- Equipment changes ability mappings (weapon swap)
- Context-sensitive abilities (different actions in different modes)
- Dynamic hotbar systems