# Attribute Menu Widget Controller Implementation Guide

Last updated: 2024-12-19

## Overview

This guide explains how to implement the Attribute Menu Widget Controller pattern using a single-instance (singleton-like) approach where the controller is cached by the HUD and accessed via a Blueprint Function Library. This pattern enables Attribute Menu widgets to initialize themselves in Event Construct without relying on the Overlay widget for initialization.

## What is the Single-Instance Controller Pattern?

### Pattern Benefits

- **Performance**: Single controller instance prevents duplicate ASC delegate bindings
- **Memory Efficiency**: One controller serves multiple widgets of the same type
- **Data Consistency**: All widgets see identical state from the same controller
- **Decoupling**: Widgets can self-initialize without dependencies on other widgets
- **Simplicity**: Clear separation between controller creation (HUD) and access (Blueprint Library)

### Pattern Components

1. **HUD as Owner**: ATDHUD (or project equivalent) owns and caches controller instances
2. **Lazy Creation**: Controllers created on first access and reused thereafter
3. **Blueprint Library**: Static functions provide clean access from any widget context
4. **Self-Initialization**: Widgets call library functions in Event Construct

## Where Controllers Live: HUD Implementation

### Required HUD Properties

The HUD class needs two properties for each controller type:

```cpp
// Example for ATDHUD or similar project HUD class
UCLASS()
class RPG_TOPDOWN_API ATDHUD : public AHUD
{
    GENERATED_BODY()

public:
    // Getter method for lazy initialization
    UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(
        const FGASCoreUIWidgetControllerParams& InWidgetControllerParams);

private:
    // Controller instance cache (owned by HUD)
    UPROPERTY()
    TObjectPtr<UTDAttributeMenuWidgetController> AttributeMenuWidgetController;

    // Controller class for instantiation (set in Blueprint or C++)
    UPROPERTY(EditAnywhere, Category = "Widget Controllers")
    TSubclassOf<UTDAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;
};
```

### HUD Getter Implementation

The getter method follows the lazy initialization pattern:

```cpp
UTDAttributeMenuWidgetController* ATDHUD::GetAttributeMenuWidgetController(
    const FGASCoreUIWidgetControllerParams& InWidgetControllerParams)
{
    // Check if controller already exists
    if (AttributeMenuWidgetController == nullptr)
    {
        // Create new controller instance (owned by this HUD)
        AttributeMenuWidgetController = NewObject<UTDAttributeMenuWidgetController>(
            this, AttributeMenuWidgetControllerClass);

        // Initialize with gameplay references
        AttributeMenuWidgetController->SetWidgetControllerParams(InWidgetControllerParams);

        // Bind to ASC/AttributeSet change delegates
        AttributeMenuWidgetController->BindCallbacksToDependencies();
    }
    
    // Return cached instance (creates on first call, reuses on subsequent calls)
    return AttributeMenuWidgetController;
}
```

### Key Implementation Notes

- **NewObject Owner**: Use `this` (the HUD) as owner for proper lifetime management
- **Class Property**: `AttributeMenuWidgetControllerClass` allows Blueprint configuration
- **Parameter Struct**: Use project-specific parameter struct (e.g., `FGASCoreUIWidgetControllerParams`)
- **Initialization Order**: SetWidgetControllerParams → BindCallbacksToDependencies

## How to Add Blueprint Library Node

### Blueprint Library Class Structure

Create a static function library to expose controller access:

```cpp
UCLASS()
class RPG_TOPDOWN_API UTDWidgetBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Get Attribute Menu Widget Controller from any widget context
     * Resolves PlayerController/PlayerState/ASC/AttributeSet and returns cached controller
     * @param WorldContext - Widget or other world context object (usually 'Self')
     * @return The cached attribute menu widget controller, or nullptr if unavailable
     */
    UFUNCTION(BlueprintCallable, Category = "TD Widget Library", 
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);

    /**
     * Get HUD Widget Controller from any widget context  
     * @param WorldContext - Widget or other world context object (usually 'Self')
     * @return The cached HUD widget controller, or nullptr if unavailable
     */
    UFUNCTION(BlueprintCallable, Category = "TD Widget Library",
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContext);
};
```

### Blueprint Library Implementation Steps

The implementation follows a standard resolution pattern:

```cpp
UTDAttributeMenuWidgetController* UTDWidgetBlueprintLibrary::GetAttributeMenuWidgetController(const UObject* WorldContext)
{
    // Step 1: Resolve PlayerController from WorldContext
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
    {
        // Step 2: Get PlayerState
        APlayerState* PS = PC->GetPlayerState();
        if (!PS) return nullptr;

        // Step 3: Resolve AbilitySystemComponent (try PlayerState first)
        UAbilitySystemComponent* ASC = nullptr;
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
        {
            ASC = ASI->GetAbilitySystemComponent();
        }
        
        // Step 3b: Fallback to Pawn if PlayerState doesn't have ASC
        if (!ASC && PC->GetPawn())
        {
            if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PC->GetPawn()))
            {
                ASC = ASI->GetAbilitySystemComponent();
            }
        }
        if (!ASC) return nullptr;

        // Step 4: Get AttributeSet from ASC
        UAttributeSet* AttributeSet = ASC->GetAttributeSet(UTDAttributeSet::StaticClass());
        if (!AttributeSet) return nullptr;

        // Step 5: Compose parameters and call HUD getter
        const FGASCoreUIWidgetControllerParams WCParams(PC, PS, ASC, AttributeSet);
        
        if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
        {
            return TDHUD->GetAttributeMenuWidgetController(WCParams);
        }
    }
    
    return nullptr;
}
```

### Function Specification Details

- **Category**: Use project-consistent naming (e.g., "TD Widget Library")
- **CallInEditor**: `true` allows testing in editor contexts
- **WorldContext Meta**: Required for UE5 to resolve world and player references
- **Return Type**: Project-specific controller class, nullable for error cases
- **Static**: Blueprint Function Library functions must be static

## Blueprint Usage: Self-Initialization Pattern

### Event Construct Workflow

Attribute Menu widgets can initialize themselves using this sequence:

```
Widget Created
    ↓
Event Construct Called
    ↓
Get Attribute Menu Widget Controller (World Context: Self)
    ↓ [Returns Controller or Null]
Branch (Is Valid?)
    ├── True: Set Widget Controller → (Optional) Broadcast Initial Values
    └── False: Handle Error (retry, show loading, etc.)
```

### Step-by-Step Blueprint Setup

1. **Open Attribute Menu Widget Blueprint**
2. **Navigate to Event Construct event**
3. **Add Library Function Node**:
   - Right-click in Blueprint graph
   - Search for "Get Attribute Menu Widget Controller"
   - Add node to graph
4. **Connect WorldContext**:
   - Connect **Self** pin to **World Context** input
5. **Validate Result**:
   - Add **Is Valid** node after library function
   - Connect controller output to Is Valid input
6. **Handle Success Path**:
   - From Is Valid **True** branch, add **Set Widget Controller** node
   - Connect controller reference to Set Widget Controller input
7. **Optional: Broadcast Initial Values**:
   - From controller reference, call **Broadcast Initial Values**
   - This populates UI with current attribute values
8. **Handle Failure Path**:
   - From Is Valid **False** branch, handle gracefully
   - Options: retry with timer, show loading state, log warning

### Blueprint Graph Example

```
[Event Construct]
        ↓
[Get Attribute Menu Widget Controller] (World Context: Self)
        ↓
    [Is Valid]
    ├─True──→ [Set Widget Controller] ──→ [Broadcast Initial Values]
    └─False─→ [Print String: "Controller not ready"] ──→ [Set Timer]
```

### Essential Blueprint Patterns

**Always Validate Controller**:
```
Controller is valid? → Set Widget Controller
Controller is null? → Handle gracefully (don't crash)
```

**Optional Initial Values**:
```
Set Widget Controller → Broadcast Initial Values
```

**Error Handling**:
```
Is Valid = False → Retry with Timer or Show Placeholder
```

## Verification and Testing

### Debug via BlueprintImplementableEvent

Add a verification event to your controller class:

```cpp
// In UTDAttributeMenuWidgetController or base class
UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
void WidgetControllerSet();
```

### Blueprint Verification Flow

```
Event Construct
    ↓
Get Attribute Menu Widget Controller
    ↓ (if valid)
Set Widget Controller
    ↓
Call WidgetControllerSet (BlueprintImplementableEvent)
    ↓ (in Blueprint implementation)
Print String: "Attribute Menu Controller Successfully Set!"
```

### Testing Checklist

**Controller Creation**:
- [ ] Controller class is set in HUD Blueprint properties
- [ ] GetAttributeMenuWidgetController returns non-null on valid setup
- [ ] Same controller instance returned on multiple calls (singleton behavior)

**Parameter Resolution**:
- [ ] PlayerController resolves correctly from widget context
- [ ] PlayerState exists and is valid
- [ ] AbilitySystemComponent found on PlayerState or Pawn
- [ ] AttributeSet of correct type exists in ASC

**Widget Integration**:
- [ ] Set Widget Controller succeeds without errors
- [ ] Widget delegates bind to controller events
- [ ] Attribute changes in game update widget display
- [ ] Multiple widgets can use same controller without conflicts

**Error Handling**:
- [ ] Null controller handled gracefully (no crashes)
- [ ] Invalid WorldContext handled properly  
- [ ] Missing dependencies logged appropriately

### Debug Print Examples

```cpp
// In Blueprint or C++ - add temporary debug output
UE_LOG(LogTemp, Warning, TEXT("Controller Valid: %s"), Controller ? TEXT("True") : TEXT("False"));
UE_LOG(LogTemp, Warning, TEXT("PlayerController: %s"), PC ? *PC->GetName() : TEXT("Null"));
UE_LOG(LogTemp, Warning, TEXT("ASC: %s"), ASC ? *ASC->GetName() : TEXT("Null"));
```

## Important Notes

### Controller Class Requirements

For Blueprint accessibility, controller classes need specific specifiers:

```cpp
// Controller must be BlueprintType and Blueprintable for Blueprint usage
UCLASS(BlueprintType, Blueprintable)
class RPG_TOPDOWN_API UTDAttributeMenuWidgetController : public UGASCoreUIWidgetController
{
    GENERATED_BODY()
    // Implementation...
};
```

**Required Specifiers**:
- `BlueprintType`: Allows Blueprint variables of this type
- `Blueprintable`: Enables Blueprint class creation from this C++ class
- `GENERATED_BODY()`: Required for UE5 reflection system

### HUD Class Assignment

Ensure your GameMode or GameState sets the correct HUD class:

```cpp
// In GameMode constructor or Blueprint
HUDClass = ATDHUD::StaticClass();
```

### Controller Class Configuration

In your HUD Blueprint (or C++ constructor):

```cpp
// Set controller classes for proper instantiation
HUDWidgetControllerClass = UTDHUDWidgetController::StaticClass();
AttributeMenuWidgetControllerClass = UTDAttributeMenuWidgetController::StaticClass();
```

### Timing Considerations

**Widget vs HUD Initialization**:
- Widgets may construct before HUD is fully initialized
- Always handle null returns from Blueprint Library functions
- Consider retry mechanisms for early widget creation

**Multiplayer Behavior**:
- Each client has its own HUD and controller instances
- Controllers are not replicated (local UI only)
- Attribute data comes from replicated ASC/AttributeSet

### Performance Notes

- **Lightweight Calls**: Blueprint Library functions are fast lookups
- **Single Delegate Binding**: Singleton pattern prevents duplicate ASC subscriptions
- **Memory Efficient**: One controller instance regardless of widget count
- **Cache Friendly**: HUD-cached controllers are quickly accessible

## Related Documentation

- [Attribute Menu Node Usage Guide](../../blueprint-library/attribute-menu-node-usage.md) - API reference and troubleshooting
- [Attribute Menu Widget Controller Setup](./attribute-menu-widget-controller-setup.md) - Detailed setup and configuration
- [Widget Controller Access Guide](../../blueprint-library/widget-controller-access-guide.md) - General Blueprint Library patterns  
- [Widget Controllers Singletons FAQ](../../../faq/widget-controllers-singletons.md) - Common questions and solutions