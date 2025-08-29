# Controller Broadcasting from Map: Iteration Walkthrough

Last updated: 2025-01-02

## Overview

This document provides a detailed, step-by-step narrative walkthrough of how the Widget Controller iterates over the AttributeSet's GameplayTag → Attribute registry and broadcasts attribute information to the UI. This supplements the comprehensive [Attribute Map Deep Dive](./attribute-map-deep-dive.md) with focused implementation details and troubleshooting guidance.

**Focus**: Controller-side implementation details, iteration patterns, error handling, and common issues.

## Prerequisites

Before reading this document, ensure you understand:
- The [Attribute Map Deep Dive](./attribute-map-deep-dive.md) concepts and patterns
- Basic [Broadcast and Binding System](./broadcast-and-binding.md) architecture
- Widget Controller pattern from [UI Widget Controller](../../ui-widget-controller.md)

## Controller Broadcasting Walkthrough

### Step 1: Registry Access and Validation

The controller first obtains a reference to the AttributeSet's registry and validates all dependencies.

```cpp
// Example pseudocode - Registry access with comprehensive validation
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    // Phase 1: Dependency Validation
    if (!AttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeSet is null - cannot broadcast any values"));
        return;
    }
    
    if (!AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeInfoDataAsset is null - cannot look up attribute metadata"));
        return;
    }
    
    // Phase 2: Type Casting and Validation
    const UTDAttributeSet* TDAttributeSet = Cast<UTDAttributeSet>(AttributeSet);
    if (!TDAttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeSet is not UTDAttributeSet type - expected UTDAttributeSet, got %s"), 
               AttributeSet ? *AttributeSet->GetClass()->GetName() : TEXT("NULL"));
        return;
    }
    
    // Phase 3: Registry Access
    const TMap<FGameplayTag, UTDAttributeSet::FAttrGetter>& Registry = TDAttributeSet->GetAttributeFunctionRegistry();
    
    // Phase 4: Registry State Validation
    if (Registry.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attribute function registry is empty - no attributes registered"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Starting broadcast for %d registered attributes"), Registry.Num());
    
    // Continue to iteration phase...
}
```

**Key Validation Points**:
1. **AttributeSet Existence**: Must have valid AttributeSet reference
2. **Data Asset Availability**: Required for looking up attribute metadata  
3. **Type Correctness**: AttributeSet must be the expected project-specific type
4. **Registry Population**: Registry must contain entries to iterate over

### Step 2: Registry Iteration Pattern

The controller iterates over each registry entry, extracting the GameplayTag and function pointer.

```cpp
// Example pseudocode - Registry iteration with structured unwrapping
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    // ... previous validation steps
    
    int32 SuccessfulBroadcasts = 0;
    int32 FailedBroadcasts = 0;
    
    // Phase 5: Registry Iteration
    for (const auto& RegistryPair : Registry)
    {
        // Structured binding (C++17) - cleaner syntax
        const FGameplayTag& AttributeTag = RegistryPair.Key;
        const UTDAttributeSet::FAttrGetter& AttributeGetter = RegistryPair.Value;
        
        // Alternative: Traditional pair access
        // const FGameplayTag& AttributeTag = RegistryPair.Key;
        // const UTDAttributeSet::FAttrGetter& AttributeGetter = RegistryPair.Value;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Processing registry entry for tag: %s"), 
               *AttributeTag.ToString());
        
        // Continue to function pointer execution...
        if (ProcessSingleAttribute(AttributeTag, AttributeGetter))
        {
            SuccessfulBroadcasts++;
        }
        else
        {
            FailedBroadcasts++;
        }
    }
    
    // Summary logging
    UE_LOG(LogTemp, Log, TEXT("Broadcast complete: %d successful, %d failed"), 
           SuccessfulBroadcasts, FailedBroadcasts);
}
```

**Iteration Best Practices**:
- **Structured Binding**: Use modern C++ syntax for cleaner code
- **Progress Tracking**: Count successful and failed broadcasts
- **Verbose Logging**: Log each registry entry for debugging
- **Error Recovery**: Continue processing even if individual entries fail

### Step 3: Function Pointer Execution

For each registry entry, the controller executes the function pointer to obtain the FGameplayAttribute.

```cpp
// Example pseudocode - Function pointer execution with error handling
bool UTDAttributeMenuWidgetController::ProcessSingleAttribute(
    const FGameplayTag& AttributeTag, 
    const UTDAttributeSet::FAttrGetter& AttributeGetter)
{
    // Phase 6: Function Pointer Validation
    if (!AttributeGetter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Null function pointer for attribute tag: %s"), 
               *AttributeTag.ToString());
        return false;
    }
    
    // Phase 7: Function Pointer Execution
    FGameplayAttribute GameplayAttr;
    try
    {
        // Direct function call through pointer
        GameplayAttr = AttributeGetter();
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Function pointer executed for %s"), 
               *AttributeTag.ToString());
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception executing function pointer for %s: %s"), 
               *AttributeTag.ToString(), *FString(e.what()));
        return false;
    }
    
    // Phase 8: Attribute Validation
    if (!GameplayAttr.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid FGameplayAttribute returned for tag: %s"), 
               *AttributeTag.ToString());
        return false;
    }
    
    // Continue to value extraction...
    return ExtractAndBroadcastValue(AttributeTag, GameplayAttr);
}
```

**Execution Safety Measures**:
- **Null Pointer Check**: Validate function pointer before calling
- **Exception Handling**: Catch and log any execution errors
- **Return Value Validation**: Ensure FGameplayAttribute is valid
- **Detailed Logging**: Track execution success/failure for each attribute

### Step 4: Value Extraction

The controller uses the FGameplayAttribute to extract the current numeric value from the AttributeSet.

```cpp
// Example pseudocode - Value extraction with safety checks
bool UTDAttributeMenuWidgetController::ExtractAndBroadcastValue(
    const FGameplayTag& AttributeTag, 
    const FGameplayAttribute& GameplayAttr)
{
    // Phase 9: Value Extraction
    float CurrentValue = 0.0f;
    
    try
    {
        // Get numeric value using the attribute
        CurrentValue = AttributeSet->GetNumericAttribute(GameplayAttr);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Extracted value %f for attribute %s"), 
               CurrentValue, *AttributeTag.ToString());
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception extracting value for %s: %s"), 
               *AttributeTag.ToString(), *FString(e.what()));
        return false;
    }
    
    // Phase 10: Value Validation (Optional)
    if (!FMath::IsFinite(CurrentValue))
    {
        UE_LOG(LogTemp, Warning, TEXT("Non-finite value %f for attribute %s"), 
               CurrentValue, *AttributeTag.ToString());
        // You might choose to continue with 0.0f or return false
        CurrentValue = 0.0f;
    }
    
    // Continue to broadcasting...
    return BroadcastSingleAttribute(AttributeTag, CurrentValue);
}
```

**Value Extraction Considerations**:
- **Exception Safety**: Handle potential exceptions from GetNumericAttribute
- **Value Validation**: Check for NaN, infinity, or other invalid values
- **Logging**: Track extracted values for debugging
- **Error Recovery**: Decide how to handle invalid values

### Step 5: Broadcasting Execution

Finally, the controller broadcasts the attribute information using the existing helper function.

```cpp
// Example pseudocode - Broadcasting with comprehensive error handling
bool UTDAttributeMenuWidgetController::BroadcastSingleAttribute(
    const FGameplayTag& AttributeTag, 
    float CurrentValue)
{
    // Phase 11: Data Asset Lookup
    FTDAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag, true);
    
    // Phase 12: Lookup Validation
    if (!AttributeInfo.AttributeTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("No UAttributeInfo entry found for tag: %s"), 
               *AttributeTag.ToString());
        return false;
    }
    
    // Phase 13: Runtime Value Assignment
    AttributeInfo.AttributeValue = CurrentValue;
    
    // Phase 14: Delegate Broadcasting
    OnAttributeInfoChanged.Broadcast(AttributeInfo);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Successfully broadcasted %s with value %f"), 
           *AttributeInfo.AttributeTag.ToString(), AttributeInfo.AttributeValue);
    
    return true;
}
```

**Broadcasting Success Factors**:
- **Data Asset Entry**: Attribute must exist in UAttributeInfo data asset
- **Valid Tag**: AttributeInfo must have valid tag after lookup
- **Delegate State**: OnAttributeInfoChanged delegate must be properly initialized
- **Widget Binding**: At least one widget should be bound to receive broadcasts

## Complete Iteration Flow Diagram

```
Start: BroadcastInitialValues()
│
├─ Validate Dependencies
│  ├─ AttributeSet exists? ──→ No ──→ Log Error & Return
│  ├─ AttributeInfoDataAsset exists? ──→ No ──→ Log Error & Return
│  └─ Cast to UTDAttributeSet successful? ──→ No ──→ Log Error & Return
│
├─ Get Registry Reference
│  ├─ Registry has entries? ──→ No ──→ Log Warning & Return
│  └─ Log registry entry count
│
├─ For Each Registry Entry:
│  ├─ Extract Tag and Function Pointer
│  ├─ Validate Function Pointer ──→ Null ──→ Log Warning & Continue
│  ├─ Execute Function Pointer ──→ Exception ──→ Log Error & Continue
│  ├─ Validate FGameplayAttribute ──→ Invalid ──→ Log Warning & Continue
│  ├─ Extract Current Value ──→ Exception ──→ Log Error & Continue
│  ├─ Lookup AttributeInfo ──→ Not Found ──→ Log Warning & Continue
│  ├─ Set Runtime Value
│  └─ Broadcast to Widgets
│
└─ Log Summary (Success/Failure Counts)
```

## Detailed Error Handling Patterns

### Pattern 1: Graceful Degradation

Instead of stopping on first error, continue processing other attributes:

```cpp
// Example pseudocode - Graceful error handling
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    // ... setup and validation
    
    TArray<FGameplayTag> FailedTags;
    
    for (const auto& [AttributeTag, AttributeGetter] : Registry)
    {
        if (!ProcessSingleAttribute(AttributeTag, AttributeGetter))
        {
            FailedTags.Add(AttributeTag);
        }
    }
    
    // Report failed attributes at the end
    if (FailedTags.Num() > 0)
    {
        FString FailedTagsString = FString::Join(FailedTags, TEXT(", "), 
            [](const FGameplayTag& Tag) { return Tag.ToString(); });
        
        UE_LOG(LogTemp, Warning, TEXT("Failed to broadcast %d attributes: %s"), 
               FailedTags.Num(), *FailedTagsString);
    }
}
```

### Pattern 2: Early Termination on Critical Errors

For critical errors, stop processing immediately:

```cpp
// Example pseudocode - Critical error handling
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    // Critical errors that should stop all processing
    if (!AttributeSet || !AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical dependencies missing - aborting all broadcasts"));
        return; // Don't process anything
    }
    
    // Non-critical errors can continue processing
    for (const auto& RegistryPair : Registry)
    {
        // Individual attribute failures don't stop the loop
    }
}
```

### Pattern 3: Retry Logic

For transient errors, implement simple retry logic:

```cpp
// Example pseudocode - Simple retry pattern
bool UTDAttributeMenuWidgetController::ProcessSingleAttributeWithRetry(
    const FGameplayTag& AttributeTag,
    const UTDAttributeSet::FAttrGetter& AttributeGetter,
    int32 MaxRetries = 2)
{
    for (int32 Attempt = 0; Attempt <= MaxRetries; ++Attempt)
    {
        if (ProcessSingleAttribute(AttributeTag, AttributeGetter))
        {
            if (Attempt > 0)
            {
                UE_LOG(LogTemp, Log, TEXT("Succeeded on attempt %d for %s"), 
                       Attempt + 1, *AttributeTag.ToString());
            }
            return true;
        }
        
        if (Attempt < MaxRetries)
        {
            UE_LOG(LogTemp, Log, TEXT("Retrying %s (attempt %d/%d)"), 
                   *AttributeTag.ToString(), Attempt + 1, MaxRetries + 1);
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed after %d attempts for %s"), 
           MaxRetries + 1, *AttributeTag.ToString());
    return false;
}
```

## Performance Optimization Strategies

### Strategy 1: Registry Caching

Cache the registry reference to avoid repeated function calls:

```cpp
// Example pseudocode - Registry caching
class UTDAttributeMenuWidgetController : public UCoreWidgetController
{
private:
    const TMap<FGameplayTag, UTDAttributeSet::FAttrGetter>* CachedRegistryPtr = nullptr;
    
    void CacheRegistryReference()
    {
        if (AttributeSet)
        {
            const UTDAttributeSet* TDAttributeSet = Cast<UTDAttributeSet>(AttributeSet);
            if (TDAttributeSet)
            {
                CachedRegistryPtr = &TDAttributeSet->GetAttributeFunctionRegistry();
                UE_LOG(LogTemp, Log, TEXT("Cached registry reference with %d entries"), 
                       CachedRegistryPtr->Num());
            }
        }
    }
    
public:
    virtual void BroadcastInitialValues() override
    {
        if (!CachedRegistryPtr)
        {
            CacheRegistryReference();
        }
        
        if (!CachedRegistryPtr || CachedRegistryPtr->Num() == 0)
        {
            return;
        }
        
        // Use cached reference instead of repeated function calls
        for (const auto& RegistryPair : *CachedRegistryPtr)
        {
            // Process attributes...
        }
    }
};
```

### Strategy 2: Batch Processing

Group multiple attribute updates together:

```cpp
// Example pseudocode - Batch processing
void UTDAttributeMenuWidgetController::BroadcastBatch(const TArray<FGameplayTag>& AttributeTags)
{
    if (!CachedRegistryPtr)
    {
        CacheRegistryReference();
    }
    
    TArray<FTDAttributeInfo> BatchedAttributeInfos;
    BatchedAttributeInfos.Reserve(AttributeTags.Num());
    
    // Process all attributes first
    for (const FGameplayTag& AttributeTag : AttributeTags)
    {
        if (const UTDAttributeSet::FAttrGetter* AttributeGetter = CachedRegistryPtr->Find(AttributeTag))
        {
            FGameplayAttribute GameplayAttr = (*AttributeGetter)();
            float CurrentValue = AttributeSet->GetNumericAttribute(GameplayAttr);
            
            FTDAttributeInfo AttributeInfo = AttributeInfoDataAsset->FindAttributeInfoForTag(AttributeTag);
            AttributeInfo.AttributeValue = CurrentValue;
            
            BatchedAttributeInfos.Add(AttributeInfo);
        }
    }
    
    // Broadcast all at once (if you have a batch delegate)
    // OnBatchAttributeInfoChanged.Broadcast(BatchedAttributeInfos);
    
    // Or broadcast individually but in rapid succession
    for (const FTDAttributeInfo& AttributeInfo : BatchedAttributeInfos)
    {
        OnAttributeInfoChanged.Broadcast(AttributeInfo);
    }
}
```

### Strategy 3: Selective Broadcasting

Only broadcast attributes that have actually changed:

```cpp
// Example pseudocode - Change tracking
class UTDAttributeMenuWidgetController : public UCoreWidgetController
{
private:
    TMap<FGameplayTag, float> LastBroadcastValues;
    
public:
    void BroadcastChangedAttributes()
    {
        for (const auto& [AttributeTag, AttributeGetter] : *CachedRegistryPtr)
        {
            FGameplayAttribute GameplayAttr = AttributeGetter();
            float CurrentValue = AttributeSet->GetNumericAttribute(GameplayAttr);
            
            float* LastValue = LastBroadcastValues.Find(AttributeTag);
            
            // Only broadcast if value has changed
            if (!LastValue || !FMath::IsNearlyEqual(*LastValue, CurrentValue))
            {
                BroadcastAttributeInfo(AttributeTag, CurrentValue);
                LastBroadcastValues.Add(AttributeTag, CurrentValue);
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("Value changed for %s: %f -> %f"), 
                       *AttributeTag.ToString(), LastValue ? *LastValue : 0.0f, CurrentValue);
            }
        }
    }
};
```

## FAQ: Common Issues and Solutions

### Q: Registry is empty when BroadcastInitialValues is called

**A: Registry Initialization Timing Issue**

**Problem**: The AttributeSet's registry hasn't been initialized when the controller tries to access it.

**Debugging Steps**:
1. Check AttributeSet constructor - does it call `InitializeAttributeFunctionRegistry()`?
2. Verify GameplayTags are properly initialized before registry setup
3. Confirm AttributeSet is fully constructed before controller accesses it

**Solution**:
```cpp
// Example pseudocode - Ensure registry is initialized
UTDAttributeSet::UTDAttributeSet()
{
    // Ensure GameplayTags are available
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // Initialize registry in constructor
    InitializeAttributeFunctionRegistry();
    
    // Log registry state for debugging
    UE_LOG(LogTemp, Log, TEXT("AttributeSet initialized with %d registry entries"), 
           AttributeFunctionRegistry.Num());
}
```

### Q: Tags don't match between registry and widgets

**A: Tag Mismatch Issue**

**Problem**: Widget AttributeTags don't exactly match the tags used in the registry.

**Debugging Steps**:
1. Log all registry tags during initialization
2. Log all widget tags during widget setup
3. Compare logs to find mismatches

**Solution**:
```cpp
// Example pseudocode - Tag validation utility
void ValidateTagConsistency()
{
    const UTDAttributeSet* AttributeSet = GetAttributeSet();
    const auto& Registry = AttributeSet->GetAttributeFunctionRegistry();
    
    UE_LOG(LogTemp, Log, TEXT("=== Registry Tags ==="));
    for (const auto& [Tag, Getter] : Registry)
    {
        UE_LOG(LogTemp, Log, TEXT("Registry: %s"), *Tag.ToString());
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== Widget Tags ==="));
    for (UTextValueButtonRow* Row : GetAllAttributeRows())
    {
        UE_LOG(LogTemp, Log, TEXT("Widget: %s"), *Row->GetAttributeTag().ToString());
    }
}
```

### Q: No UAttributeInfo entry found for specific tags

**A: Data Asset Configuration Issue**

**Problem**: The UAttributeInfo data asset is missing entries for some attributes in the registry.

**Debugging Steps**:
1. Check data asset in editor - ensure all attributes are configured
2. Verify tag names match exactly (case-sensitive)
3. Confirm data asset reference is set in controller

**Solution**:
```cpp
// Example pseudocode - Data asset validation
void ValidateAttributeInfoAsset()
{
    if (!AttributeInfoDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("AttributeInfoDataAsset is null"));
        return;
    }
    
    const UTDAttributeSet* AttributeSet = GetAttributeSet();
    const auto& Registry = AttributeSet->GetAttributeFunctionRegistry();
    
    TArray<FGameplayTag> MissingTags;
    
    for (const auto& [RegistryTag, Getter] : Registry)
    {
        FTDAttributeInfo Info = AttributeInfoDataAsset->FindAttributeInfoForTag(RegistryTag, false);
        if (!Info.AttributeTag.IsValid())
        {
            MissingTags.Add(RegistryTag);
        }
    }
    
    if (MissingTags.Num() > 0)
    {
        for (const FGameplayTag& MissingTag : MissingTags)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing AttributeInfo entry for: %s"), 
                   *MissingTag.ToString());
        }
    }
}
```

### Q: Broadcasts succeed but widgets don't update

**A: Widget Binding Issue**

**Problem**: Widgets aren't properly bound to the controller's delegate.

**Debugging Steps**:
1. Verify OnWidgetControllerSet is called on child widgets
2. Check that delegate binding succeeds without errors
3. Confirm widget filtering logic (MatchesTagExact) is correct

**Solution**:
```cpp
// Example pseudocode - Debug binding state
void UTextValueButtonRow::OnWidgetControllerSet_Implementation()
{
    UTDAttributeMenuWidgetController* AttributeController = 
        Cast<UTDAttributeMenuWidgetController>(WidgetController);
    
    if (AttributeController)
    {
        // Log binding attempt
        UE_LOG(LogTemp, Log, TEXT("Binding widget %s to controller delegate"), 
               *GetName());
        
        AttributeController->OnAttributeInfoChanged.AddDynamic(
            this, &UTextValueButtonRow::OnAttributeInfoReceived);
        
        // Verify binding succeeded
        if (AttributeController->OnAttributeInfoChanged.IsBound())
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully bound to delegate"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to bind to delegate"));
        }
    }
}

void UTextValueButtonRow::OnAttributeInfoReceived(const FTDAttributeInfo& AttributeInfo)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("Widget %s received broadcast for %s (my tag: %s)"), 
           *GetName(), *AttributeInfo.AttributeTag.ToString(), *AttributeTag.ToString());
    
    if (AttributeInfo.AttributeTag.MatchesTagExact(AttributeTag))
    {
        UE_LOG(LogTemp, Log, TEXT("Widget %s updating UI for matching tag %s"), 
               *GetName(), *AttributeTag.ToString());
        UpdateAttributeDisplay(AttributeInfo);
    }
}
```

### Q: Controller is null or wrong type

**A: Controller Setup Issue**

**Problem**: Widget doesn't receive valid controller reference or gets wrong controller type.

**Debugging Steps**:
1. Verify HUD properly creates controller before widgets
2. Check controller type in SetWidgetController calls
3. Ensure controller initialization order is correct

**Solution**:
```cpp
// Example pseudocode - Robust controller setup
void APlayerHUD::ShowAttributeMenu()
{
    // Create controller FIRST
    UTDAttributeMenuWidgetController* Controller = GetAttributeMenuWidgetController();
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create AttributeMenuWidgetController"));
        return;
    }
    
    // Create widget
    AttributeMenuWidget = CreateWidget<UAttributeMenuWidget>(GetOwningPlayerController(), AttributeMenuClass);
    if (!AttributeMenuWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create AttributeMenuWidget"));
        return;
    }
    
    // Set controller with validation
    AttributeMenuWidget->SetWidgetController(Controller);
    
    // Broadcast initial values AFTER widget setup
    Controller->BroadcastInitialValues();
    
    // Add to viewport
    AttributeMenuWidget->AddToViewport();
}
```

### Q: Performance issues with frequent broadcasts

**A: Broadcasting Frequency Issue**

**Problem**: Too many broadcasts causing UI performance problems.

**Solutions**:
1. **Implement throttling** (limit update frequency)
2. **Use change detection** (only broadcast when values actually change)
3. **Batch updates** (group multiple changes together)
4. **Selective updates** (only update visible UI elements)

```cpp
// Example pseudocode - Performance optimizations
class UTDAttributeMenuWidgetController : public UCoreWidgetController
{
private:
    float UpdateInterval = 0.1f; // 10 updates per second max
    float LastUpdateTime = 0.0f;
    TMap<FGameplayTag, float> CachedValues;
    
public:
    void BroadcastInitialValues() override
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Throttle updates
        if (CurrentTime - LastUpdateTime < UpdateInterval)
        {
            return;
        }
        
        // Only broadcast changed values
        for (const auto& [AttributeTag, AttributeGetter] : *CachedRegistryPtr)
        {
            float CurrentValue = GetAttributeValue(AttributeTag, AttributeGetter);
            
            float* CachedValue = CachedValues.Find(AttributeTag);
            if (!CachedValue || !FMath::IsNearlyEqual(*CachedValue, CurrentValue))
            {
                BroadcastAttributeInfo(AttributeTag, CurrentValue);
                CachedValues.Add(AttributeTag, CurrentValue);
            }
        }
        
        LastUpdateTime = CurrentTime;
    }
};
```

## Related Documentation

- [Attribute Map Deep Dive](./attribute-map-deep-dive.md) - Comprehensive tutorial on registry patterns
- [Broadcast and Binding System](./broadcast-and-binding.md) - Core broadcasting architecture
- [Scalable Broadcasting Plan](./scalable-broadcasting-plan.md) - Strategy comparison and trade-offs
- [Attribute Map FAQ](./faq-attribute-map.md) - Common questions and troubleshooting