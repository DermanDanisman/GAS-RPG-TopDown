# Troubleshooting: Input tags → Ability activation

Last updated: 2025-09-03

This guide helps diagnose and fix common issues with the input tag → ability activation flow.

## Symptoms and fixes

### 1) No input callbacks fire in controller
**Symptoms:**
- Pressing LMB/RMB/1-4 does nothing
- No on-screen debug messages from controller callbacks

**Common causes:**
- Default Input Component Class not set correctly
- Input mapping context not assigned or added
- InputConfig asset not configured

**Solutions:**
1. Check Project Settings → Input → Default Input Component Class = UTDEnhancedInputComponent
2. Verify ATDPlayerController Blueprint has:
   - GASInputMappingContext assigned
   - InputConfig assigned
3. Ensure BeginPlay adds the mapping context:
   ```cpp
   EISubsystem->AddMappingContext(GASInputMappingContext, 0);
   ```
4. Check Enhanced Input assets exist and are properly mapped

### 2) Callbacks fire, but ASC doesn't react
**Symptoms:**
- Controller debug shows input tags
- No ability activation occurs

**Common causes:**
- ASC is null or not initialized
- GetASC() returns null early in game
- ASC not properly set up on pawn

**Solutions:**
1. Add null checks in controller callbacks:
   ```cpp
   void ATDPlayerController::AbilityInputActionHeld(FGameplayTag InputTag)
   {
       if (UTDAbilitySystemComponent* ASC = GetASC())
       {
           ASC->AbilityInputTagHeld(InputTag);
       }
       else
       {
           UE_LOG(LogTemp, Warning, TEXT("ASC is null for input: %s"), *InputTag.ToString());
       }
   }
   ```
2. Verify InitAbilityActorInfo was called on the ASC
3. Check that the pawn has the correct ASC component

### 3) ASC reacts, but no ability activates
**Symptoms:**
- ASC methods are called
- Spec.GetDynamicSpecSourceTags() doesn't contain expected InputTag

**Common causes:**
- StartupInputTag not set on ability CDO
- Ability not granted with proper input tag
- Tag name mismatch

**Solutions:**
1. Verify ability's StartupInputTag is set in Blueprint/CDO:
   ```cpp
   // In ability Blueprint, set StartupInputTag to exact FTDGameplayTags value
   StartupInputTag = FTDGameplayTags::Get().InputTag_LMB;
   ```
2. Check UTDAbilityInitComponent grants abilities correctly
3. Debug Spec.GetDynamicSpecSourceTags() in ASC methods
4. Ensure exact tag matching (not parent tags)

### 4) Ability activates repeatedly each frame
**Symptoms:**
- Ability fires continuously while holding input
- Performance issues from repeated activation

**Common causes:**
- Ability ends immediately, so Held loop re-activates
- No duration or blocking tags on ability

**Solutions:**
1. Add minimum duration to ability:
   ```cpp
   // In ability Blueprint or C++, set Duration > 0
   Duration = 0.1f; // Or appropriate value
   ```
2. Use blocking tags to prevent re-activation
3. Add internal state checks in ability logic

### 5) QuickSlot keys work, but mouse buttons don't
**Symptoms:**
- Keyboard 1-4 activate abilities
- LMB/RMB do nothing

**Common causes:**
- Input mapping context missing mouse bindings
- UI capturing mouse input
- Wrong input mode set

**Solutions:**
1. Verify IMC has LMB/RMB mappings:
   - IA_LMB → Mouse Left Button
   - IA_RMB → Mouse Right Button
2. Set appropriate input mode in controller:
   ```cpp
   SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
   ```
3. Check UI widgets aren't consuming mouse events

### 6) Dedicated server testing: inputs do nothing
**Symptoms:**
- Works in standalone/PIE
- No abilities activate on dedicated server

**Common causes:**
- Abilities not granted on authority
- Client prediction issues
- Network replication problems

**Solutions:**
1. Verify UTDAbilityInitComponent checks HasAuthority before granting
2. Ensure ability execution policies allow client prediction
3. Check ASC replication settings
4. Test that InitAbilityActorInfo is called on both client and server

## Useful debug techniques

### On-screen debug in controller
```cpp
void ATDPlayerController::AbilityInputActionHeld(FGameplayTag InputTag)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Green, 
            FString::Printf(TEXT("Input Held: %s"), *InputTag.ToString()));
    }
    
    if (UTDAbilitySystemComponent* ASC = GetASC())
    {
        ASC->AbilityInputTagHeld(InputTag);
    }
}
```

### ASC debugging
```cpp
void UGASCoreAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid InputTag received"));
        return;
    }
    
    bool bFoundMatch = false;
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
    {
        if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
        {
            bFoundMatch = true;
            UE_LOG(LogTemp, Log, TEXT("Found ability spec for tag: %s"), *InputTag.ToString());
            // ... rest of activation logic
        }
    }
    
    if (!bFoundMatch)
    {
        UE_LOG(LogTemp, Warning, TEXT("No ability found for InputTag: %s"), *InputTag.ToString());
    }
}
```

### Gameplay Debugger
- Enable with apostrophe (') key
- Shows ability system state, active abilities, and tags
- Useful for inspecting ASC state in real-time

### Blueprint debugging
- Add print statements in ability Blueprints
- Check ActivateAbility node execution
- Verify input tag values match expectations

## Common tag naming issues

### Wrong tag format
```cpp
// Wrong - will not match
FGameplayTag WrongTag = FGameplayTag::RequestGameplayTag(FName("InputTag.QuickSlot-1"));

// Correct - matches centralized tags
const FTDGameplayTags& Tags = FTDGameplayTags::Get();
FGameplayTag CorrectTag = Tags.InputTag_QuickSlot_1; // "InputTag.QuickSlot1"
```

### Parent tag confusion
```cpp
// This will NOT work with HasTagExact
FGameplayTag ParentTag = FGameplayTag::RequestGameplayTag(FName("InputTag"));

// This WILL work
FGameplayTag ExactTag = FTDGameplayTags::Get().InputTag_LMB; // "InputTag.LMB"
```

## Verification checklist

When setting up or debugging, verify:
- [ ] FTDGameplayTags::InitializeNativeGameplayTags() called at startup
- [ ] Default Input Component Class = UTDEnhancedInputComponent
- [ ] ATDPlayerController has InputConfig and GASInputMappingContext assigned
- [ ] Enhanced Input assets exist and are mapped correctly
- [ ] Abilities have StartupInputTag set to exact FTDGameplayTags values
- [ ] UTDAbilityInitComponent grants abilities on authority
- [ ] ASC is properly initialized (InitAbilityActorInfo called)
- [ ] Input mapping context is added in BeginPlay
- [ ] Tags use exact matching (HasTagExact vs MatchesTag)

## Still having issues?

1. Enable verbose logging for GameplayAbilities
2. Use breakpoints in controller callbacks and ASC methods
3. Compare working vs non-working ability setups
4. Check console for warnings about missing tags or assets
5. Test in different network configurations (standalone vs client/server)