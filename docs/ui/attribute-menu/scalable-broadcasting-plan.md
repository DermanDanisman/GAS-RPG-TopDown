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

## Recommendation and Implementation Path

### Recommended Approach: Strategy 2 (Tag Lists)

For most projects, **Strategy 2 (Tag List Configuration)** provides the best balance of:
- **Maintainability**: Clear, organized structure
- **Designer Control**: Configurable without code changes  
- **Performance**: Minimal overhead during broadcasting
- **Scalability**: Supports growth to 50+ attributes
- **Debugging**: Easy to understand and troubleshoot

### Implementation Phases

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

### Migration Strategy

**Gradual Migration**:
1. **Keep Existing Code**: Don't remove hardcoded broadcasts immediately
2. **Add New System**: Implement tag list approach alongside existing code
3. **Feature Flag**: Use boolean property to switch between old/new broadcasting
4. **Test Thoroughly**: Validate identical behavior between approaches
5. **Remove Legacy**: Delete hardcoded broadcasts once new system is validated

**Validation Steps**:
- Compare broadcast timing between old and new approaches
- Verify all attributes are broadcast in both systems
- Test with various attribute counts and UI configurations
- Performance profile both approaches under load
- Validate multiplayer behavior remains consistent

## Future Considerations

### Extensibility Planning

**Additional Attribute Types**:
- Combat attributes (damage bonuses, resistances)
- Skill attributes (crafting bonuses, gathering rates)
- Social attributes (reputation, faction standing)
- Temporary attributes (buffs, debuffs with expiration)

**Dynamic Attribute Systems**:
- Runtime attribute creation for modding support
- Attribute templates for different character types
- Conditional attribute visibility based on player progression
- Localized attribute names and descriptions

### Performance Optimization

**Caching Strategies**:
- Cache tag-to-attribute mappings at controller initialization
- Pre-calculate frequently-accessed attribute information
- Batch attribute updates to minimize broadcast overhead
- Implement selective broadcasting for visible-only attributes

**Memory Management**:
- Optimize data asset loading and caching
- Minimize delegate binding overhead
- Consider object pooling for frequently-created widgets
- Profile memory usage with large attribute sets

### Architecture Evolution

**Event-Driven Broadcasting**:
- Move from pull-based (controller queries) to push-based (AttributeSet broadcasts)
- Implement attribute change batching for multiple simultaneous updates
- Add priority-based broadcasting for critical vs. informational attributes
- Support for attribute change animations and transitions

**Modding Support**:
- Plugin-based attribute registration
- Runtime attribute discovery and broadcasting
- Scriptable attribute behavior and formatting
- Community-contributed attribute types and UI layouts

## Related Documentation

- [Attribute Tags and Binding Pattern](./attribute-tags-and-binding.md) - Implementation details for widget tag assignment and filtering
- [Attribute Menu Widget Controller Setup](./attribute-menu-widget-controller-setup.md) - Current controller implementation with hardcoded broadcasts
- [Broadcast and Binding System](./broadcast-and-binding.md) - Overview of the delegate-based broadcasting architecture
- [Attribute Info Data Asset](../data-asset/attribute-info.md) - Data asset structure and configuration for attribute metadata
- [Attributes Gameplay Tags](../../../systems/attributes-gameplay-tags.md) - GameplayTag definitions and registration patterns