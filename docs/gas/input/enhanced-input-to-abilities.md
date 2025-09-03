# Enhanced Input → Ability Mapping (Data-Driven) — TD Input Config

Last updated: 2025-09-03

This guide covers creating and configuring UTDInputConfig assets to map Enhanced Input actions to gameplay tags for ability activation. Use with UTDEnhancedInputComponent for complete input binding.

## Overview
- UTDInputConfig extends UGASCoreAbilityInputConfig (data asset)
- Maps UInputAction assets to FGameplayTag values
- Used by UTDEnhancedInputComponent to bind callbacks automatically
- Supports runtime lookup via FindAbilityInputActionByTag (hierarchy-friendly)

## Creating Enhanced Input assets

### Step 1: Create Input Actions
Create UInputAction assets for each input you want to bind:

1. Content Browser → Right-click → Input → Input Action
2. Create assets with descriptive names:
   - `IA_LMB` (Left Mouse Button)
   - `IA_RMB` (Right Mouse Button) 
   - `IA_QuickSlot1` through `IA_QuickSlot4`
3. For each action:
   - Set Value Type to Axis1D (float)
   - Leave other settings at defaults
4. Save in organized folder (e.g., Content/Input/Actions/)

### Step 2: Create Input Mapping Context
Create a UInputMappingContext to define key mappings:

1. Content Browser → Right-click → Input → Input Mapping Context
2. Name it descriptively (e.g., `IMC_GASAbilities`)
3. Add mappings for each action:
   - `IA_LMB` → Left Mouse Button
   - `IA_RMB` → Right Mouse Button  
   - `IA_QuickSlot1` → One
   - `IA_QuickSlot2` → Two
   - `IA_QuickSlot3` → Three
   - `IA_QuickSlot4` → Four
4. Save the context

## Creating the UTDInputConfig data asset

### Step 1: Create the data asset
1. Content Browser → Right-click → Miscellaneous → Data Asset
2. Select `UTDInputConfig` as the class
3. Name it descriptively (e.g., `DA_GASInputConfig`)

### Step 2: Configure ability input actions
Open the data asset and populate the Ability Input Actions array:

```
[0] Input Action: IA_LMB, Input Tag: InputTag.LMB
[1] Input Action: IA_RMB, Input Tag: InputTag.RMB  
[2] Input Action: IA_QuickSlot1, Input Tag: InputTag.QuickSlot1
[3] Input Action: IA_QuickSlot2, Input Tag: InputTag.QuickSlot2
[4] Input Action: IA_QuickSlot3, Input Tag: InputTag.QuickSlot3
[5] Input Action: IA_QuickSlot4, Input Tag: InputTag.QuickSlot4
```

**Important:** Use the exact tag names from FTDGameplayTags. The editor should auto-complete these when you start typing "InputTag".

## Blueprint setup

### ATDPlayerController configuration
Open your ATDPlayerController Blueprint and set the following defaults:

1. **GAS Input Mapping Context**: Assign your IMC_GASAbilities asset
2. **Input Config**: Assign your DA_GASInputConfig asset  
3. **Move Action**: Assign movement input action if using movement

### Project settings
Ensure Project Settings → Input → Default Input Component Class is set to `UTDEnhancedInputComponent`.

## Verification checklist
- [ ] All Enhanced Input assets created and saved
- [ ] Input Mapping Context has correct key mappings
- [ ] UTDInputConfig data asset has all required InputAction ↔ InputTag pairs
- [ ] InputTags use exact names from FTDGameplayTags (InputTag.LMB not InputTag_LMB)
- [ ] ATDPlayerController Blueprint has assets assigned
- [ ] Default Input Component Class is UTDEnhancedInputComponent

## Usage in code
The input config supports lookup by tag:

```cpp
const UInputAction* UTDInputConfig::FindAbilityInputActionByTag(const FGameplayTag& InputTag) const
{
    // Uses MatchesTag for hierarchy support
    for (const FGASCoreAbilityInputAction& Action : AbilityInputActions)
    {
        if (Action.InputTag.MatchesTag(InputTag))
        {
            return Action.InputAction;
        }
    }
    return nullptr;
}
```

## Common issues
- **Tags not showing in editor**: Ensure FTDGameplayTags::InitializeNativeGameplayTags() is called at startup
- **Input not responding**: Check that mapping context is added in BeginPlay and Default Input Component Class is correct
- **Wrong input triggered**: Verify InputTag names match exactly between config and ability StartupInputTags

## Runtime remapping
To change input mappings at runtime, modify the FGameplayAbilitySpec::GetDynamicSpecSourceTags() rather than the input config. See the runtime remapping guide for details.

Related guides:
- Binding callbacks: ../input/td-enhanced-input-component.md
- Ability activation: ../abilities/ability-input-tags-and-activation.md  
- Runtime remapping: ../howto/runtime-remapping.md