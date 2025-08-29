# Scalable Broadcasting Plan

Last updated: 2025-01-02

## Overview

This document outlines strategies for evolving from hardcoded per-attribute broadcasting to scalable, data-driven approaches. The current implementation manually broadcasts each attribute individually, leading to code duplication and maintenance burden. The goal is to describe patterns for iterating over attributes programmatically while maintaining the existing tag-based filtering system.

**Important**: This document provides planning and pseudocode only. Actual implementation is out-of-scope for this documentation-only initiative.

## Current Problem: Hardcoded Broadcasting

### Naive Implementation Example

The current controller implementation requires manual broadcasting for each attribute:

```cpp
// Current approach: Hardcoded per-attribute broadcasts
void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

    // Manual broadcast for each primary attribute
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Strength, 
                          TDAttributeSet->GetStrength());
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Intelligence, 
                          TDAttributeSet->GetIntelligence());
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Resilience, 
                          TDAttributeSet->GetResilience());
    BroadcastAttributeInfo(GameplayTags.Attributes_Primary_Vigor, 
                          TDAttributeSet->GetVigor());

    // Manual broadcast for each secondary attribute  
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_Armor, 
                          TDAttributeSet->GetArmor());
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_ArmorPenetration, 
                          TDAttributeSet->GetArmorPenetration());
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_BlockChance, 
                          TDAttributeSet->GetBlockChance());
    BroadcastAttributeInfo(GameplayTags.Attributes_Secondary_CriticalHitChance, 
                          TDAttributeSet->GetCriticalHitChance());
    
    // ... Continue for dozens of attributes
}
```

### Problems with Current Approach

**Code Duplication**: Each attribute requires identical broadcasting pattern
- Same `BroadcastAttributeInfo()` call structure
- Same tag-to-value mapping logic
- Repeated patterns across primary, secondary, vital attributes

**Maintenance Burden**: Adding new attributes requires code changes
- Must remember to add broadcast call for each new attribute
- Easy to forget attributes during development
- Controller changes required for designer-added attributes

**Scalability Issues**: Doesn't scale well for large attribute sets
- 20+ attributes = 20+ manual broadcast calls
- Difficult to organize and maintain as project grows
- Hard to ensure all attributes are covered

**Error Prone**: Manual approach leads to mistakes
- Easy to use wrong tag/getter combination
- Typos in method names or tag references
- Missing attributes discovered only during testing

## Strategic Goals

### Data-Driven Iteration

**Objective**: Loop over attributes programmatically rather than hardcoding each one
- Eliminate manual per-attribute broadcast calls
- Reduce code duplication to a single loop structure
- Enable automatic discovery of new attributes

**Benefits**:
- Adding new attributes requires no controller code changes
- Consistent broadcasting behavior for all attributes
- Single source of truth for attribute broadcasting logic
- Less error-prone than manual enumeration

### Maintainable Architecture

**Objective**: Create broadcasting system that scales with project growth
- Support for attribute categories (Primary, Secondary, Vital)
- Flexible enough to handle future attribute types
- Clear separation between data definition and broadcasting logic

**Benefits**:
- Designers can add attributes without programmer intervention
- Consistent behavior across all attribute types
- Easier to debug and maintain broadcasting logic
- Future-proof for project expansion

### Performance Optimization

**Objective**: Maintain or improve broadcasting performance
- Avoid reflection overhead where possible
- Minimize data asset lookups during broadcasting
- Cache frequently-accessed attribute information

**Benefits**:
- Fast UI initialization times
- Minimal impact on gameplay performance  
- Scalable to hundreds of attributes if needed

## Strategy 1: Data Asset Iteration

### Approach Overview

Iterate through the UAttributeInfo data asset entries and broadcast each attribute found. This leverages the existing data asset infrastructure without requiring additional data structures.

### Pseudocode Implementation

```cpp
// Strategy 1: Iterate through AttributeInfo data asset entries
void UTDAttributeMenuWidgetController::BroadcastInitialValues_DataDriven()
{
    if (!AttributeInfoDataAsset || !AttributeSet)
    {
        return;
    }

    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);

    // Get all attribute info entries from data asset
    for (const FAuraAttributeInfo& AttributeInfo : AttributeInfoDataAsset->GetAttributeInformation())
    {
        // Get current value from AttributeSet using tag lookup
        float CurrentValue = GetAttributeValueByTag(AttributeInfo.AttributeTag);
        
        // Broadcast using existing pattern
        BroadcastAttributeInfo(AttributeInfo.AttributeTag, CurrentValue);
    }
}

// Helper function: Map tags to AttributeSet getter methods
float UTDAttributeMenuWidgetController::GetAttributeValueByTag(const FGameplayTag& AttributeTag)
{
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // Map tags to getter methods
    if (AttributeTag.MatchesTagExact(GameplayTags.Attributes_Primary_Strength))
        return TDAttributeSet->GetStrength();
    if (AttributeTag.MatchesTagExact(GameplayTags.Attributes_Primary_Intelligence))
        return TDAttributeSet->GetIntelligence();
    // ... Continue for all attributes
    
    // Default return for unhandled tags
    UE_LOG(LogTemp, Warning, TEXT("No getter found for attribute tag: %s"), 
           *AttributeTag.ToString());
    return 0.0f;
}
```

### Strategy 1 Trade-offs

**Advantages**:
- ✅ Leverages existing data asset infrastructure
- ✅ No additional data structures needed
- ✅ Automatic discovery of new attributes added to data asset
- ✅ Consistent with current architecture

**Disadvantages**:
- ❌ Still requires manual tag-to-getter mapping in helper function
- ❌ GetAttributeValueByTag becomes large switch/if chain
- ❌ Adding new attributes still requires code changes (in helper function)
- ❌ Doesn't eliminate all hardcoded attribute handling

**Best For**: 
- Projects with moderate numbers of attributes (10-30)
- Teams comfortable with maintaining tag-to-getter mappings
- Situations where data asset is the primary source of attribute metadata

## Strategy 2: Tag List Configuration

### Approach Overview

Create configured lists of attribute tags grouped by category, then iterate through these lists to perform broadcasts. This provides more structure than pure data asset iteration while remaining data-driven.

### Pseudocode Implementation

```cpp
// Strategy 2: Configured tag lists for different attribute categories
class UTDAttributeMenuWidgetController : public UGASCoreUIWidgetController
{
protected:
    /** Primary attribute tags for broadcasting */
    UPROPERTY(EditAnywhere, Category = "Attribute Broadcasting")
    TArray<FGameplayTag> PrimaryAttributeTags;
    
    /** Secondary attribute tags for broadcasting */  
    UPROPERTY(EditAnywhere, Category = "Attribute Broadcasting")
    TArray<FGameplayTag> SecondaryAttributeTags;
    
    /** Vital attribute tags for broadcasting */
    UPROPERTY(EditAnywhere, Category = "Attribute Broadcasting")
    TArray<FGameplayTag> VitalAttributeTags;
};

void UTDAttributeMenuWidgetController::BroadcastInitialValues_ConfiguredLists()
{
    if (!AttributeSet)
    {
        return;
    }

    // Broadcast all primary attributes
    BroadcastAttributeCategory(PrimaryAttributeTags);
    
    // Broadcast all secondary attributes  
    BroadcastAttributeCategory(SecondaryAttributeTags);
    
    // Broadcast all vital attributes
    BroadcastAttributeCategory(VitalAttributeTags);
}

void UTDAttributeMenuWidgetController::BroadcastAttributeCategory(const TArray<FGameplayTag>& AttributeTags)
{
    for (const FGameplayTag& AttributeTag : AttributeTags)
    {
        float CurrentValue = GetAttributeValueByTag(AttributeTag);
        BroadcastAttributeInfo(AttributeTag, CurrentValue);
    }
}
```

### Configuration Example

```cpp
// In Blueprint or data configuration
PrimaryAttributeTags = [
    "Attributes.Primary.Strength",
    "Attributes.Primary.Intelligence", 
    "Attributes.Primary.Resilience",
    "Attributes.Primary.Vigor"
]

SecondaryAttributeTags = [
    "Attributes.Secondary.Armor",
    "Attributes.Secondary.ArmorPenetration",
    "Attributes.Secondary.BlockChance", 
    "Attributes.Secondary.CriticalHitChance",
    "Attributes.Secondary.CriticalHitDamage",
    "Attributes.Secondary.CriticalHitResistance",
    "Attributes.Secondary.HealthRegeneration",
    "Attributes.Secondary.ManaRegeneration",
    "Attributes.Secondary.MaxHealth",
    "Attributes.Secondary.MaxMana"
]
```

### Strategy 2 Trade-offs

**Advantages**:
- ✅ Clear separation of attribute categories
- ✅ Designer-configurable without code changes
- ✅ Organized, maintainable structure
- ✅ Can disable/enable categories easily
- ✅ Supports different broadcasting rules per category

**Disadvantages**:
- ❌ Still requires GetAttributeValueByTag helper function
- ❌ Configuration must be maintained in multiple places
- ❌ Risk of tags being in wrong category
- ❌ Additional configuration complexity

**Best For**:
- Projects with clear attribute category separation
- Teams wanting designer control over broadcasting behavior
- Situations where different categories need different handling
- Large projects with 50+ attributes needing organization

## Strategy 3: Reflection-Based Approach

### Approach Overview

Use Unreal Engine's reflection system to automatically discover AttributeSet getter methods and their corresponding GameplayTag associations. This provides the most automated approach with minimal configuration.

### Pseudocode Implementation

```cpp
// Strategy 3: Reflection-based automatic discovery
void UTDAttributeMenuWidgetController::BroadcastInitialValues_Reflection()
{
    if (!AttributeSet)
    {
        return;
    }

    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    UClass* AttributeSetClass = TDAttributeSet->GetClass();
    
    // Iterate through all properties in the AttributeSet
    for (TFieldIterator<FProperty> PropIt(AttributeSetClass); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        
        // Check if this is a GameplayAttribute property
        if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            if (StructProp->Struct == FGameplayAttribute::StaticStruct())
            {
                // Extract attribute name and map to tag
                FString AttributeName = Property->GetName();
                FGameplayTag AttributeTag = MapAttributeNameToTag(AttributeName);
                
                if (AttributeTag.IsValid())
                {
                    // Get current value using the attribute
                    FGameplayAttribute GameplayAttr(Property);
                    float CurrentValue = AttributeSet->GetNumericAttribute(GameplayAttr);
                    
                    // Broadcast using discovered information
                    BroadcastAttributeInfo(AttributeTag, CurrentValue);
                }
            }
        }
    }
}

FGameplayTag UTDAttributeMenuWidgetController::MapAttributeNameToTag(const FString& AttributeName)
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // Map attribute property names to GameplayTags
    static const TMap<FString, FGameplayTag> AttributeNameToTagMap = {
        {"StrengthAttribute", GameplayTags.Attributes_Primary_Strength},
        {"IntelligenceAttribute", GameplayTags.Attributes_Primary_Intelligence},
        {"ResilienceAttribute", GameplayTags.Attributes_Primary_Resilience},
        {"VigorAttribute", GameplayTags.Attributes_Primary_Vigor},
        {"ArmorAttribute", GameplayTags.Attributes_Secondary_Armor},
        // ... Continue for all attributes
    };
    
    if (const FGameplayTag* FoundTag = AttributeNameToTagMap.Find(AttributeName))
    {
        return *FoundTag;
    }
    
    return FGameplayTag::EmptyTag;
}
```

### Strategy 3 Trade-offs  

**Advantages**:
- ✅ Automatically discovers new attributes without configuration
- ✅ No manual broadcasting code for individual attributes
- ✅ Directly uses AttributeSet property information
- ✅ Most scalable approach for large attribute sets
- ✅ Eliminates getter method mapping

**Disadvantages**:
- ❌ More complex implementation requiring reflection knowledge
- ❌ Still requires name-to-tag mapping configuration
- ❌ Potential performance overhead from reflection
- ❌ Harder to debug when things go wrong
- ❌ Less predictable execution order

**Best For**:
- Large projects with 100+ attributes
- Teams comfortable with reflection systems
- Situations where automatic discovery is critical
- Performance-tolerant contexts (reflection overhead acceptable)

## Strategy 4: Hybrid Data Asset + Reflection

### Approach Overview

Combine data asset information with reflection capabilities to get the best of both worlds: structured attribute metadata with automatic value retrieval.

### Pseudocode Implementation

```cpp
// Strategy 4: Hybrid approach using data asset + reflection
void UTDAttributeMenuWidgetController::BroadcastInitialValues_Hybrid()
{
    if (!AttributeInfoDataAsset || !AttributeSet)
    {
        return;
    }

    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    // Create tag-to-attribute mapping using reflection once
    TMap<FGameplayTag, FGameplayAttribute> TagToAttributeMap;
    BuildTagToAttributeMap(TagToAttributeMap);
    
    // Iterate through data asset entries (provides metadata)
    for (const FAuraAttributeInfo& AttributeInfo : AttributeInfoDataAsset->GetAttributeInformation())
    {
        // Find corresponding GameplayAttribute using reflection
        if (const FGameplayAttribute* GameplayAttr = TagToAttributeMap.Find(AttributeInfo.AttributeTag))
        {
            // Get current value directly from attribute
            float CurrentValue = AttributeSet->GetNumericAttribute(*GameplayAttr);
            
            // Broadcast using existing pattern
            BroadcastAttributeInfo(AttributeInfo.AttributeTag, CurrentValue);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No GameplayAttribute found for tag: %s"), 
                   *AttributeInfo.AttributeTag.ToString());
        }
    }
}

void UTDAttributeMenuWidgetController::BuildTagToAttributeMap(TMap<FGameplayTag, FGameplayAttribute>& OutTagToAttributeMap)
{
    // This would be built once and cached
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    OutTagToAttributeMap.Add(GameplayTags.Attributes_Primary_Strength, TDAttributeSet->GetStrengthAttribute());
    OutTagToAttributeMap.Add(GameplayTags.Attributes_Primary_Intelligence, TDAttributeSet->GetIntelligenceAttribute());
    OutTagToAttributeMap.Add(GameplayTags.Attributes_Primary_Resilience, TDAttributeSet->GetResilienceAttribute());
    OutTagToAttributeMap.Add(GameplayTags.Attributes_Primary_Vigor, TDAttributeSet->GetVigorAttribute());
    // ... Continue for all attributes
    
    // Alternatively, use reflection to build this mapping automatically
}
```

### Strategy 4 Trade-offs

**Advantages**:
- ✅ Leverages existing data asset structure for metadata
- ✅ Uses reflection for efficient value retrieval  
- ✅ No manual getter method calls
- ✅ Automatic discovery of new data asset entries
- ✅ More predictable than pure reflection

**Disadvantages**:
- ❌ Most complex implementation combining multiple systems
- ❌ Still requires tag-to-attribute mapping construction
- ❌ Higher initial setup complexity
- ❌ Debugging complexity from multiple systems

**Best For**:
- Projects needing both metadata richness and automatic discovery
- Teams comfortable with hybrid architectural approaches
- Situations where data asset provides valuable additional information
- Performance-critical contexts where getter method calls are expensive

## Strategy 5: GameplayTag→Attribute Registry

### Approach Overview

Create a registry within the AttributeSet that maps FGameplayTag to attribute accessors, eliminating per-attribute boilerplate in the Widget Controller. The registry provides a centralized lookup mechanism where the Widget Controller can iterate over tag-to-attribute mappings without hardcoding individual attribute broadcasts.

This approach moves the mapping logic into the AttributeSet itself, making it the authoritative source for both attribute storage and attribute access patterns.

### Approach 5A: Delegate-Based Registry

The initial approach uses delegates to provide a callable interface for attribute retrieval:

```cpp
// In AttributeSet header - delegate signature for attribute access
DECLARE_DELEGATE_RetVal(FGameplayAttribute, FAttributeAccessorDelegate);

class UTDAttributeSet : public UGASCoreAttributeSet
{
private:
    /** Registry mapping GameplayTags to attribute accessor delegates */
    TMap<FGameplayTag, FAttributeAccessorDelegate> AttributeAccessorRegistry;
    
    /** Initialize the registry during construction */
    void InitializeAttributeRegistry();

public:
    /** Get the attribute registry for external iteration */
    const TMap<FGameplayTag, FAttributeAccessorDelegate>& GetAttributeRegistry() const 
    { 
        return AttributeAccessorRegistry; 
    }
    
    // ... existing attribute definitions
};

// In AttributeSet implementation
void UTDAttributeSet::InitializeAttributeRegistry()
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // Register primary attributes
    AttributeAccessorRegistry.Add(
        GameplayTags.Attributes_Primary_Strength,
        FAttributeAccessorDelegate::CreateUObject(this, &UTDAttributeSet::GetStrengthAttribute)
    );
    
    AttributeAccessorRegistry.Add(
        GameplayTags.Attributes_Primary_Intelligence,
        FAttributeAccessorDelegate::CreateUObject(this, &UTDAttributeSet::GetIntelligenceAttribute)
    );
    
    // Register secondary attributes
    AttributeAccessorRegistry.Add(
        GameplayTags.Attributes_Secondary_Armor,
        FAttributeAccessorDelegate::CreateUObject(this, &UTDAttributeSet::GetArmorAttribute)
    );
    
    // ... Continue for all attributes
}
```

Widget Controller usage with delegate approach:

```cpp
// In AttributeMenuWidgetController
void UTDAttributeMenuWidgetController::BroadcastInitialValues_Registry()
{
    if (!AttributeSet)
    {
        return;
    }

    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const TMap<FGameplayTag, FAttributeAccessorDelegate>& Registry = TDAttributeSet->GetAttributeRegistry();

    // Iterate over registry entries
    for (const auto& RegistryPair : Registry)
    {
        const FGameplayTag& AttributeTag = RegistryPair.Key;
        const FAttributeAccessorDelegate& AccessorDelegate = RegistryPair.Value;
        
        // Execute delegate to get FGameplayAttribute
        FGameplayAttribute GameplayAttr = AccessorDelegate.ExecuteIfBound();
        
        // Get current value using the attribute
        float CurrentValue = AttributeSet->GetNumericAttribute(GameplayAttr);
        
        // Broadcast using existing pattern
        BroadcastAttributeInfo(AttributeTag, CurrentValue);
    }
}
```

### Approach 5B: Function Pointer Registry (Refined)

The refined approach replaces delegates with static function pointers for better performance and simpler syntax:

```cpp
// Function pointer type for attribute getters
using FAttrGetter = FGameplayAttribute(*)();

class UTDAttributeSet : public UGASCoreAttributeSet  
{
private:
    /** Registry mapping GameplayTags to static function pointers */
    TMap<FGameplayTag, FAttrGetter> AttributeFunctionRegistry;
    
    /** Initialize the function pointer registry */
    void InitializeAttributeFunctionRegistry();

public:
    /** Get the function pointer registry for external iteration */
    const TMap<FGameplayTag, FAttrGetter>& GetAttributeFunctionRegistry() const
    {
        return AttributeFunctionRegistry; 
    }
    
    // Static accessor functions for registry
    static FGameplayAttribute GetStrengthAttributeStatic() { return GetStrengthAttribute(); }
    static FGameplayAttribute GetIntelligenceAttributeStatic() { return GetIntelligenceAttribute(); }
    static FGameplayAttribute GetArmorAttributeStatic() { return GetArmorAttribute(); }
    // ... Continue for all attributes
    
    // ... existing attribute definitions
};

// In AttributeSet implementation  
void UTDAttributeSet::InitializeAttributeFunctionRegistry()
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // Register function pointers for primary attributes
    AttributeFunctionRegistry.Add(GameplayTags.Attributes_Primary_Strength, &GetStrengthAttributeStatic);
    AttributeFunctionRegistry.Add(GameplayTags.Attributes_Primary_Intelligence, &GetIntelligenceAttributeStatic);
    
    // Register function pointers for secondary attributes  
    AttributeFunctionRegistry.Add(GameplayTags.Attributes_Secondary_Armor, &GetArmorAttributeStatic);
    
    // ... Continue for all attributes
}
```

Widget Controller usage with function pointer approach:

```cpp
// In AttributeMenuWidgetController
void UTDAttributeMenuWidgetController::BroadcastInitialValues_FunctionRegistry()
{
    if (!AttributeSet)
    {
        return;
    }

    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const TMap<FGameplayTag, FAttrGetter>& Registry = TDAttributeSet->GetAttributeFunctionRegistry();

    // Iterate over registry entries  
    for (const auto& RegistryPair : Registry)
    {
        const FGameplayTag& AttributeTag = RegistryPair.Key;
        const FAttrGetter& AttributeGetter = RegistryPair.Value;
        
        // Call function pointer to get FGameplayAttribute
        FGameplayAttribute GameplayAttr = AttributeGetter();
        
        // Get current value using the attribute
        float CurrentValue = AttributeSet->GetNumericAttribute(GameplayAttr);
        
        // Broadcast using existing pattern
        BroadcastAttributeInfo(AttributeTag, CurrentValue);
    }
}
```

### Alternative: Member Function Pointer Approach

For scenarios where static functions are not preferred, member function pointers provide another option:

```cpp
// Member function pointer type
using FAttrMemberGetter = FGameplayAttribute(UTDAttributeSet::*)() const;

class UTDAttributeSet : public UGASCoreAttributeSet
{
private:
    /** Registry mapping GameplayTags to member function pointers */
    TMap<FGameplayTag, FAttrMemberGetter> AttributeMemberFunctionRegistry;

public:
    const TMap<FGameplayTag, FAttrMemberGetter>& GetAttributeMemberFunctionRegistry() const
    {
        return AttributeMemberFunctionRegistry;
    }
};

// Usage in Widget Controller
void UTDAttributeMenuWidgetController::BroadcastInitialValues_MemberFunctionRegistry()
{
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    const TMap<FGameplayTag, FAttrMemberGetter>& Registry = TDAttributeSet->GetAttributeMemberFunctionRegistry();

    for (const auto& RegistryPair : Registry)
    {
        const FGameplayTag& AttributeTag = RegistryPair.Key;
        const FAttrMemberGetter& MemberGetter = RegistryPair.Value;
        
        // Call member function pointer
        FGameplayAttribute GameplayAttr = (TDAttributeSet->*MemberGetter)();
        
        float CurrentValue = AttributeSet->GetNumericAttribute(GameplayAttr);
        BroadcastAttributeInfo(AttributeTag, CurrentValue);
    }
}
```

### Registry Initialization Patterns

**Constructor Initialization**:
```cpp
UTDAttributeSet::UTDAttributeSet()
{
    InitializeAttributeFunctionRegistry();
}
```

**Lazy Initialization**:
```cpp
const TMap<FGameplayTag, FAttrGetter>& UTDAttributeSet::GetAttributeFunctionRegistry() const
{
    if (AttributeFunctionRegistry.Num() == 0)
    {
        // const_cast acceptable for lazy initialization
        const_cast<UTDAttributeSet*>(this)->InitializeAttributeFunctionRegistry();
    }
    return AttributeFunctionRegistry;
}
```

**Static Initialization (Shared Registry)**:
```cpp
// Static registry shared across all instances
static TMap<FGameplayTag, FAttrGetter> StaticAttributeRegistry;

const TMap<FGameplayTag, FAttrGetter>& UTDAttributeSet::GetStaticAttributeRegistry()
{
    static bool bInitialized = false;
    if (!bInitialized)
    {
        InitializeStaticAttributeRegistry();
        bInitialized = true;
    }
    return StaticAttributeRegistry;
}
```

### Strategy 5 Trade-offs

**Approach 5A (Delegates) Advantages**:
- ✅ Familiar Unreal delegate patterns
- ✅ Type-safe callable interface  
- ✅ Supports member function binding
- ✅ Memory management handled by delegate system
- ✅ Can capture additional context if needed

**Approach 5A (Delegates) Disadvantages**:
- ❌ Higher memory overhead per registry entry
- ❌ Slightly more complex syntax for execution
- ❌ Potential for delegate invalidation if AttributeSet is destroyed
- ❌ More setup code required for delegate creation

**Approach 5B (Function Pointers) Advantages**:
- ✅ Minimal memory footprint (just function addresses)
- ✅ Maximum performance - direct function calls
- ✅ Simple, direct calling syntax
- ✅ No lifetime management concerns
- ✅ Clean separation of concerns

**Approach 5B (Function Pointers) Disadvantages**:
- ❌ Requires static wrapper functions for each attribute
- ❌ Less flexible than delegates for future extensions
- ❌ Static functions cannot access instance data easily
- ❌ More boilerplate code for static wrappers

**Overall Strategy 5 Benefits**:
- ✅ **Centralized Mapping**: All attribute access logic contained in AttributeSet
- ✅ **Eliminates Widget Controller Boilerplate**: No per-attribute broadcasting code needed
- ✅ **Type Safety**: Compile-time verification of attribute accessors
- ✅ **Maintainability**: Adding new attributes only requires registry entries
- ✅ **Performance**: Direct attribute access without string-based lookups
- ✅ **Scalability**: Registry approach scales linearly with attribute count

**Overall Strategy 5 Disadvantages**:
- ❌ **Setup Complexity**: Requires careful registry initialization
- ❌ **AttributeSet Coupling**: Widget Controller becomes dependent on AttributeSet registry format
- ❌ **Memory Usage**: Registry storage adds overhead compared to hardcoded approach
- ❌ **Debugging Complexity**: Indirect calls through registry can complicate debugging

**Best For**:
- Projects with 30+ attributes where hardcoded approaches become unwieldy
- Teams prioritizing maintainability and seeking to eliminate boilerplate code
- Architectures where AttributeSet should be the authoritative source for attribute access patterns
- Performance-sensitive contexts where function pointer calls are preferred over reflection
- Long-term projects where attribute sets are expected to grow significantly

**Comparison with Other Strategies**:
- **vs Strategy 1 (Data Asset)**: More type-safe, better performance, but requires registry setup
- **vs Strategy 2 (Tag Lists)**: More automated, eliminates configuration maintenance
- **vs Strategy 3 (Reflection)**: Better performance, more predictable, but requires more setup code
- **vs Strategy 4 (Hybrid)**: Simpler architecture, more centralized, but less metadata flexibility

## Recommendation and Implementation Path

### Recommended Approach by Project Scale

**Small to Medium Projects (10-30 attributes): Strategy 2 (Tag List Configuration)**
- **Maintainability**: Clear, organized structure
- **Designer Control**: Configurable without code changes  
- **Performance**: Minimal overhead during broadcasting
- **Debugging**: Easy to understand and troubleshoot

**Large Projects (30+ attributes): Strategy 5B (Function Pointer Registry)**
- **Scalability**: Handles unlimited attribute growth
- **Maintainability**: Eliminates all Widget Controller boilerplate
- **Performance**: Direct function calls with minimal overhead
- **Centralization**: AttributeSet becomes single source of truth for access patterns

**Performance-Critical Projects: Strategy 5B (Function Pointer Registry)**
- **Maximum Performance**: Direct function pointer calls
- **Minimal Memory**: Only function addresses stored
- **Type Safety**: Compile-time verification of all accessors
- **Predictable Execution**: No reflection or string-based lookups

### Implementation Phases

**Strategy 2 Implementation (Tag Lists)**:

**Phase 1: Proof of Concept**
- Create single category tag list (Primary attributes only)
- Implement basic iteration loop
- Test with existing UI system
- Validate performance impact

**Phase 2: Category Expansion** 
- Add Secondary and Vital attribute tag lists
- Implement category-specific broadcasting logic
- Test with full attribute set
- Optimize performance bottlenecks

**Phase 3: Configuration Polish**
- Move tag lists to data asset or config files
- Add Blueprint configuration support
- Implement validation and error handling
- Create documentation for designers

**Phase 4: Advanced Features**
- Add selective broadcasting (primary-only mode)
- Implement attribute category filtering
- Support runtime attribute addition/removal
- Add performance profiling and optimization

**Strategy 5 Implementation (Registry-Based)**:

**Phase 1: Registry Foundation**
- Choose approach (5A Delegates vs 5B Function Pointers vs Member Function Pointers)
- Implement basic registry data structure in AttributeSet
- Create registry initialization method
- Test registry population with subset of attributes

**Phase 2: Accessor Integration**
- Add static wrapper functions (for 5B approach) or delegate setup (for 5A approach)  
- Populate registry with all primary attributes
- Implement Widget Controller registry iteration
- Validate identical behavior to hardcoded approach

**Phase 3: Full Attribute Coverage**
- Add secondary and vital attributes to registry
- Test with complete attribute set
- Performance profile vs existing hardcoded approach
- Implement error handling for missing registry entries

**Phase 4: Production Hardening**
- Add registry validation and diagnostic logging
- Implement lazy vs eager initialization options
- Create unit tests for registry correctness
- Document registry extension patterns for new attributes

**Phase 5: Advanced Registry Features**
- Support for conditional attribute registration
- Category-based registry subsets
- Runtime registry modification capabilities  
- Integration with existing data asset systems

### Migration Strategy

**Gradual Migration (Strategy 2)**:
1. **Keep Existing Code**: Don't remove hardcoded broadcasts immediately
2. **Add New System**: Implement tag list approach alongside existing code
3. **Feature Flag**: Use boolean property to switch between old/new broadcasting
4. **Test Thoroughly**: Validate identical behavior between approaches
5. **Remove Legacy**: Delete hardcoded broadcasts once new system is validated

**Registry Migration (Strategy 5)**:
1. **Registry Preparation**: Implement registry infrastructure without changing broadcasting
2. **Parallel Implementation**: Add registry-based broadcasting as alternative method
3. **Validation Phase**: Run both approaches side-by-side with comparison logging
4. **Gradual Switchover**: Enable registry-based approach on specific attribute categories first
5. **Legacy Cleanup**: Remove hardcoded approach once registry is fully validated

**Validation Steps (All Strategies)**:
- Compare broadcast timing between old and new approaches
- Verify all attributes are broadcast in both systems
- Test with various attribute counts and UI configurations
- Performance profile both approaches under load
- Validate multiplayer behavior remains consistent
- Ensure identical attribute values are broadcast by both methods

**Rollback Strategy (Strategy 5)**:
- **Feature Flag**: Maintain ability to switch back to hardcoded approach
- **Registry Validation**: Add runtime checks to verify registry completeness
- **Fallback Mechanism**: If registry entry is missing, fall back to hardcoded value
- **Diagnostic Logging**: Log registry misses and performance differences

```cpp
// Example feature flag implementation
UPROPERTY(EditAnywhere, Category = "Attribute Broadcasting")
bool bUseRegistryBroadcasting = false;

void UTDAttributeMenuWidgetController::BroadcastInitialValues()
{
    if (bUseRegistryBroadcasting)
    {
        BroadcastInitialValues_FunctionRegistry();
    }
    else
    {
        BroadcastInitialValues_Hardcoded(); // Legacy approach
    }
}
```

## Future Considerations

### Extensibility Planning

**Additional Attribute Types**:
- Combat attributes (damage bonuses, resistances)
- Skill attributes (crafting bonuses, gathering rates)
- Social attributes (reputation, faction standing)
- Temporary attributes (buffs, debuffs with expiration)

**Registry-Based Extensions (Strategy 5)**:
- **Conditional Registration**: Registry entries that are only active under certain conditions
- **Category Registries**: Separate registries for different attribute types (Primary, Secondary, Combat, etc.)
- **Metadata Integration**: Registry entries that include additional metadata (display formatting, UI category, etc.)
- **Runtime Registration**: Support for plugins or mods to register new attributes at runtime

```cpp
// Advanced registry with metadata
struct FAttributeRegistryEntry
{
    FGameplayTag AttributeTag;
    FAttrGetter AttributeGetter;
    FText DisplayCategory;      // For UI organization
    bool bIsVisible = true;     // For conditional display
    float DisplayPriority = 0.0f; // For sorting
};

// Conditional registry example
void UTDAttributeSet::RegisterCombatAttributes()
{
    // Only register combat attributes if combat system is enabled
    if (GetWorld()->GetGameState<ATDGameState>()->IsCombatSystemEnabled())
    {
        AttributeFunctionRegistry.Add(GameplayTags.Attributes_Combat_WeaponDamage, &GetWeaponDamageAttributeStatic);
        AttributeFunctionRegistry.Add(GameplayTags.Attributes_Combat_SpellPower, &GetSpellPowerAttributeStatic);
    }
}
```

**Dynamic Attribute Systems**:
- Runtime attribute creation for modding support
- Attribute templates for different character types
- Conditional attribute visibility based on player progression
- Localized attribute names and descriptions

**Registry Evolution Patterns**:
- **Versioned Registries**: Support for different registry versions for backwards compatibility
- **Plugin Integration**: Allow plugins to extend the base registry with additional attributes
- **Configuration-Driven**: Move registry definitions to external configuration files
- **Template-Based**: Support for attribute templates that generate registry entries automatically

### Performance Optimization

**Caching Strategies (General)**:
- Cache tag-to-attribute mappings at controller initialization
- Pre-calculate frequently-accessed attribute information
- Batch attribute updates to minimize broadcast overhead
- Implement selective broadcasting for visible-only attributes

**Registry-Specific Optimizations (Strategy 5)**:
- **Static Registry Sharing**: Use static registry shared across all AttributeSet instances
- **Registry Precomputation**: Initialize registry once at startup rather than per-instance
- **Direct Access Caching**: Cache frequently-accessed attribute function pointers
- **Category-Based Registries**: Separate registries by attribute category to reduce iteration overhead

```cpp
// Optimized registry with category separation
class UTDAttributeSet : public UGASCoreAttributeSet
{
private:
    // Separate registries for different categories
    static TMap<FGameplayTag, FAttrGetter> PrimaryAttributeRegistry;
    static TMap<FGameplayTag, FAttrGetter> SecondaryAttributeRegistry;
    static TMap<FGameplayTag, FAttrGetter> VitalAttributeRegistry;
    
public:
    // Category-specific broadcasting for better performance
    void BroadcastPrimaryAttributes(UTDAttributeMenuWidgetController* Controller);
    void BroadcastSecondaryAttributes(UTDAttributeMenuWidgetController* Controller);
    void BroadcastVitalAttributes(UTDAttributeMenuWidgetController* Controller);
};

// Performance-optimized broadcasting
void UTDAttributeMenuWidgetController::BroadcastInitialValues_Optimized()
{
    const UTDAttributeSet* TDAttributeSet = CastChecked<UTDAttributeSet>(AttributeSet);
    
    // Broadcast only requested categories to minimize work
    if (ShouldBroadcastPrimaryAttributes())
    {
        BroadcastAttributeCategory(TDAttributeSet->GetPrimaryAttributeRegistry());
    }
    
    if (ShouldBroadcastSecondaryAttributes())
    {
        BroadcastAttributeCategory(TDAttributeSet->GetSecondaryAttributeRegistry());
    }
}
```

**Memory Management**:
- Optimize data asset loading and caching
- Minimize delegate binding overhead (for Strategy 5A)
- Consider object pooling for frequently-created widgets
- Profile memory usage with large attribute sets
- Use lightweight function pointers over delegates where possible (Strategy 5B)

### Architecture Evolution

**Event-Driven Broadcasting**:
- Move from pull-based (controller queries) to push-based (AttributeSet broadcasts)
- Implement attribute change batching for multiple simultaneous updates
- Add priority-based broadcasting for critical vs. informational attributes
- Support for attribute change animations and transitions

**Registry-Driven Architecture (Strategy 5)**:
- **Centralized Attribute Management**: AttributeSet becomes single source of truth for all attribute access
- **Plugin-Based Extension**: Registry allows plugins to add attributes without modifying core files
- **Data-Driven Registration**: Move registry initialization to external configuration files
- **Automatic Discovery**: Use reflection to automatically populate registry from AttributeSet properties

```cpp
// Advanced registry-driven architecture
class UTDAttributeSet : public UGASCoreAttributeSet
{
public:
    /** Configuration-driven registry initialization */
    void InitializeRegistryFromDataAsset(UAttributeRegistryDataAsset* RegistryConfig);
    
    /** Automatic registry population using reflection */
    void InitializeRegistryFromReflection();
    
    /** Plugin-safe registry extension */
    void RegisterPluginAttributes(const TMap<FGameplayTag, FAttrGetter>& PluginAttributes);
    
    /** Category-filtered registry access */
    TMap<FGameplayTag, FAttrGetter> GetAttributeRegistryByCategory(const FGameplayTag& CategoryTag) const;
};

// Configuration data asset for registry-driven approach
UCLASS()
class UAttributeRegistryDataAsset : public UDataAsset
{
    GENERATED_BODY()
    
public:
    /** Attribute registration entries */
    UPROPERTY(EditAnywhere, Category = "Registry")
    TArray<FAttributeRegistrationEntry> RegistryEntries;
};

USTRUCT()
struct FAttributeRegistrationEntry
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, Category = "Registry")
    FGameplayTag AttributeTag;
    
    UPROPERTY(EditAnywhere, Category = "Registry")
    FString AttributeAccessorName; // Name of the static accessor function
    
    UPROPERTY(EditAnywhere, Category = "Registry")
    FGameplayTag CategoryTag;
    
    UPROPERTY(EditAnywhere, Category = "Registry")
    bool bIsEnabled = true;
};
```

**Modding Support**:
- Plugin-based attribute registration
- Runtime attribute discovery and broadcasting
- Scriptable attribute behavior and formatting
- Community-contributed attribute types and UI layouts

## Integration with AttributeSetManager

The registry-based approach can be further enhanced by integrating with an AttributeSetManager that provides centralized management of multiple AttributeSet instances and their registries:

```cpp
// AttributeSetManager.h - Centralized registry management
class UAttributeSetManager : public UObject
{
    GENERATED_BODY()
    
private:
    /** Master registry combining all AttributeSet registries */
    TMap<FGameplayTag, FAttrGetter> MasterAttributeRegistry;
    
    /** Registered AttributeSet instances */
    TArray<TWeakObjectPtr<UTDAttributeSet>> RegisteredAttributeSets;

public:
    /** Register an AttributeSet and merge its registry */
    void RegisterAttributeSet(UTDAttributeSet* AttributeSet);
    
    /** Get the combined registry from all registered AttributeSets */
    const TMap<FGameplayTag, FAttrGetter>& GetMasterRegistry() const;
    
    /** Get attribute value by tag from any registered AttributeSet */
    float GetAttributeValueByTag(const FGameplayTag& AttributeTag) const;
    
    /** Broadcast all attributes from all registered AttributeSets */
    void BroadcastAllAttributes(UTDAttributeMenuWidgetController* Controller) const;
    
    /** Get singleton instance */
    static UAttributeSetManager* Get();
};

// Widget Controller integration with AttributeSetManager
void UTDAttributeMenuWidgetController::BroadcastInitialValues_Manager()
{
    UAttributeSetManager* Manager = UAttributeSetManager::Get();
    if (!Manager)
    {
        return;
    }
    
    // Use manager's master registry for broadcasting
    const TMap<FGameplayTag, FAttrGetter>& MasterRegistry = Manager->GetMasterRegistry();
    
    for (const auto& RegistryPair : MasterRegistry)
    {
        const FGameplayTag& AttributeTag = RegistryPair.Key;
        float CurrentValue = Manager->GetAttributeValueByTag(AttributeTag);
        BroadcastAttributeInfo(AttributeTag, CurrentValue);
    }
}
```

**AttributeSetManager Benefits**:
- **Multi-AttributeSet Support**: Handle characters with multiple AttributeSet instances
- **Centralized Registry**: Single source for all attribute access across the game
- **Automatic Discovery**: Manager can automatically discover and register AttributeSets
- **Plugin Coordination**: Manage attribute registries from multiple plugins
- **Lifetime Management**: Handle AttributeSet creation/destruction and registry updates

## Strategy Decision Matrix

To help you choose the best approach for your project, use this decision matrix based on your requirements:

| Strategy | Complexity | Performance | Scalability | Maintainability | Best For |
|----------|------------|-------------|-------------|-----------------|----------|
| **Strategy 1: Data Asset Iteration** | Low | Medium | Medium | Medium | Small projects, simple attribute sets |
| **Strategy 2: GameplayTag Array** | Low | Medium | Medium | Low | Prototypes, temporary solutions |
| **Strategy 3: Reflection-Based** | High | Low | High | High | Complex projects with many attribute types |
| **Strategy 4: Hybrid Data Asset + Reflection** | Medium | Medium | High | High | Medium-large projects, good compromise |
| **Strategy 5A: Delegate Registry** | Medium | Medium | High | High | Projects prioritizing Unreal patterns |
| **Strategy 5B: Function Pointer Registry** | Medium | High | High | High | **Production recommendation** |
| **Strategy 6: Central Manager** | High | High | High | Medium | Large projects with multiple AttributeSet types |

### Decision Factors

**Choose Strategy 5B (Function Pointer Registry) if**:
- You want maximum performance with minimal memory overhead
- Your project has 10+ attributes that will grow over time
- You prefer simple, direct function calls
- You need compile-time type safety
- You want the best balance of performance and maintainability

**Choose Strategy 4 (Hybrid) if**:
- You need to support designers adding attributes via data assets
- Your project mixes hardcoded and data-driven attributes
- You want flexibility between performance and designer control

**Choose Strategy 6 (Central Manager) if**:
- You have multiple AttributeSet classes
- Your project uses plugins that add their own attributes
- You need centralized control over all attribute access

**Avoid Strategy 2 (GameplayTag Array) for**:
- Production projects (maintenance nightmare)
- Projects with frequent attribute additions
- Any project expected to scale beyond a few attributes

### Implementation Priority

1. **Start with Strategy 5B** (Function Pointer Registry) for most projects
2. **Add Strategy 6** (Central Manager) if you later need multi-AttributeSet support
3. **Consider Strategy 4** (Hybrid) if designers need to add attributes without code changes

For a comprehensive tutorial on implementing Strategy 5B, see [Attribute Map Deep Dive](./attribute-map-deep-dive.md).

## Cross-References and Deep Dive Resources

### Primary Resources

- **[Attribute Map Deep Dive](./attribute-map-deep-dive.md)** - **⭐ START HERE** - Comprehensive teacher-level tutorial covering:
  - Detailed explanation of delegate vs function pointer approaches
  - Type aliasing patterns with `using` and templated aliases
  - Step-by-step implementation guide
  - Controller-side iteration patterns
  - UI initialization order and timing
  - Common pitfalls and best practices

- **[Controller Broadcasting from Map](./controller-broadcast-from-map.md)** - Detailed walkthrough of:
  - Registry iteration implementation
  - Error handling patterns  
  - Performance optimization strategies
  - Troubleshooting common issues

- **[Attribute Map FAQ](./faq-attribute-map.md)** - Quick answers to:
  - Delegates vs function pointers trade-offs
  - Multiplayer and networking considerations
  - Performance optimization questions
  - Debugging and troubleshooting guide

### Supporting Documentation

- [Attribute Tags and Binding Pattern](./attribute-tags-and-binding.md) - Implementation details for widget tag assignment and filtering
- [Attribute Menu Widget Controller Setup](./attribute-menu-widget-controller-setup.md) - Current controller implementation with hardcoded broadcasts
- [Broadcast and Binding System](./broadcast-and-binding.md) - Overview of the delegate-based broadcasting architecture
- [Attribute Info Data Asset](../data-asset/attribute-info.md) - Data asset structure and configuration for attribute metadata
- [Attributes Gameplay Tags](../../../systems/attributes-gameplay-tags.md) - GameplayTag definitions and registration patterns

### Learning Path

**Beginner**: Start with [Broadcast and Binding System](./broadcast-and-binding.md) to understand the current architecture, then read this document for strategy overview.

**Intermediate**: Read [Attribute Map Deep Dive](./attribute-map-deep-dive.md) for comprehensive implementation guidance, then [Controller Broadcasting from Map](./controller-broadcast-from-map.md) for detailed iteration patterns.

**Advanced**: Review [Attribute Map FAQ](./faq-attribute-map.md) for edge cases and optimization strategies. Consider Strategy 6 (Central Manager) for complex multi-AttributeSet scenarios.

**Troubleshooting**: Start with [Controller Broadcasting from Map FAQ section](./controller-broadcast-from-map.md#faq-common-issues-and-solutions), then check [Attribute Map FAQ](./faq-attribute-map.md) for advanced debugging techniques.