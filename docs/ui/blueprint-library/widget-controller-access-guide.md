# Widget Controller Access Guide (Blueprint Function Library)

Last updated: 2024-12-19

## Overview

This guide demonstrates how to implement a Blueprint Function Library to access singleton-like Widget Controllers (HUD/Overlay and Attribute Menu) from any widget context. This enables widgets to initialize their own controllers in Event Construct without tight coupling to the Overlay or requiring direct HUD references.

### What is the Singleton Controller Pattern?

Widget Controllers in this system follow a singleton pattern where:
- **Single Instance**: Only one instance of each controller type exists per player
- **HUD-Cached**: Controllers are cached in the ATDHUD class
- **Lazy Creation**: Controllers are created on first access and reused thereafter
- **Centralized Access**: Blueprint Function Library provides clean access from any context

### Why Use This Pattern?

- **Decoupling**: Widgets don't need direct references to HUD or other widgets
- **Performance**: Avoids creating duplicate controllers with duplicate delegate bindings
- **Memory**: Single controller instance reduces memory overhead
- **Consistency**: All widgets access the same controller data and state
- **Flexibility**: Widgets can self-initialize in Event Construct

## Blueprint Function Library Implementation

### Class Structure

Create a Blueprint Function Library class to expose clean access nodes:

```cpp
UCLASS()
class RPG_TOPDOWN_API UTDWidgetBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Get HUD/Overlay Widget Controller from any widget context
     * @param WorldContext - Widget or other world context object (usually 'Self')
     * @return The cached HUD widget controller, or nullptr if unavailable
     */
    UFUNCTION(BlueprintCallable, Category = "TD Widget Library", 
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContext);

    /**
     * Get Attribute Menu Widget Controller from any widget context
     * @param WorldContext - Widget or other world context object (usually 'Self')
     * @return The cached attribute menu widget controller, or nullptr if unavailable
     */
    UFUNCTION(BlueprintCallable, Category = "TD Widget Library", 
              CallInEditor = true, meta = (WorldContext = "WorldContext"))
    static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);
};
```

### Node Signatures in Blueprint

When implemented, these functions will appear in Blueprint as:

- **Get HUD Widget Controller**
  - Input: World Context (Object Reference)
  - Output: TD HUD Widget Controller (Object Reference)

- **Get Attribute Menu Widget Controller** 
  - Input: World Context (Object Reference)
  - Output: TD Attribute Menu Widget Controller (Object Reference)

### WorldContext Parameter Explanation

The `WorldContext` parameter is crucial for resolving the correct world and player:

- **What it does**: Provides access to the UWorld and enables player resolution
- **Common values**: Widget 'Self' reference, PlayerController, GameMode, etc.
- **Blueprint usage**: Pass `Self` (the widget calling the function)
- **Meta tag**: `meta = (WorldContext = "WorldContext")` tells Unreal this is a context object

## Implementation Steps

### Step 1: Resolve PlayerController and PlayerState

```cpp
static UTDHUDWidgetController* UTDWidgetBlueprintLibrary::GetHUDWidgetController(const UObject* WorldContext)
{
    // Resolve the PlayerController from WorldContext
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
    {
        // Get the PlayerState (contains persistent player data)
        APlayerState* PS = PC->GetPlayerState();
        if (!PS) return nullptr;
        
        // Continue to next step...
    }
    return nullptr;
}
```

### Step 2: Resolve AbilitySystemComponent (ASC)

```cpp
// Inside the function, after getting PlayerController and PlayerState
UAbilitySystemComponent* ASC = nullptr;

// Try to get ASC from PlayerState first (common pattern)
if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
{
    ASC = ASI->GetAbilitySystemComponent();
}

// Fallback: try to get ASC from Pawn if PlayerState doesn't have it
if (!ASC)
{
    if (APawn* Pawn = PC->GetPawn())
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
        {
            ASC = ASI->GetAbilitySystemComponent();
        }
    }
}

if (!ASC) return nullptr;
```

### Step 3: Resolve AttributeSet

```cpp
// Get the AttributeSet from the ASC
UAttributeSet* AttributeSet = nullptr;
if (ASC)
{
    // Assuming you want UTDAttributeSet specifically
    AttributeSet = ASC->GetAttributeSet(UTDAttributeSet::StaticClass());
}

if (!AttributeSet) return nullptr;
```

### Step 4: Compose FGASCoreUIWidgetControllerParams

```cpp
// Create the parameter struct with all resolved references
const FGASCoreUIWidgetControllerParams WidgetControllerParams(
    PC,           // PlayerController
    PS,           // PlayerState  
    ASC,          // AbilitySystemComponent
    AttributeSet  // AttributeSet
);
```

### Step 5: Call ATDHUD Controller Methods

```cpp
// Get the HUD and call the appropriate controller method
if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
{
    // For HUD controller:
    return TDHUD->GetHUDWidgetController(WidgetControllerParams);
    
    // For Attribute Menu controller:
    // return TDHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
}

return nullptr;
```

### Complete Implementation Example

```cpp
// HUD Widget Controller Access
UTDHUDWidgetController* UTDWidgetBlueprintLibrary::GetHUDWidgetController(const UObject* WorldContext)
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
    {
        APlayerState* PS = PC->GetPlayerState();
        if (!PS) return nullptr;

        // Resolve ASC
        UAbilitySystemComponent* ASC = nullptr;
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
        {
            ASC = ASI->GetAbilitySystemComponent();
        }
        if (!ASC && PC->GetPawn())
        {
            if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PC->GetPawn()))
            {
                ASC = ASI->GetAbilitySystemComponent();
            }
        }
        if (!ASC) return nullptr;

        // Resolve AttributeSet
        UAttributeSet* AttributeSet = ASC->GetAttributeSet(UTDAttributeSet::StaticClass());
        if (!AttributeSet) return nullptr;

        // Compose parameters and call HUD
        const FGASCoreUIWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AttributeSet);
        if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
        {
            return TDHUD->GetHUDWidgetController(WidgetControllerParams);
        }
    }
    return nullptr;
}

// Attribute Menu Widget Controller Access
UTDAttributeMenuWidgetController* UTDWidgetBlueprintLibrary::GetAttributeMenuWidgetController(const UObject* WorldContext)
{
    // Same resolution logic as above, but call different HUD method
    // ... (resolution steps)
    
    if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
    {
        return TDHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
    }
    return nullptr;
}
```

## Blueprint Usage

### Self-Initializing Widget Pattern

In your Attribute Menu widget's Event Construct:

1. **Right-click** in the Blueprint graph
2. Search for "Get Attribute Menu Widget Controller"
3. Connect **Self** to the **World Context** input
4. Connect output to **Set Widget Controller** node
5. Optionally call **Broadcast Initial Values**

### Blueprint Graph Example

```
Event Construct
    ↓
Get Attribute Menu Widget Controller (World Context: Self)
    ↓ [TD Attribute Menu Widget Controller]
Set Widget Controller
    ↓ (Optional)
[Controller] → Broadcast Initial Values
```

### Widget Controller Setup Sequence

```
1. Widget Created
2. Event Construct Called
3. Get Attribute Menu Widget Controller (via Blueprint Library)
4. Set Widget Controller (connects controller to widget)
5. Broadcast Initial Values (pushes current data to UI)
6. Widget displays current attribute values
```

## Timing Considerations

### HUD Overlay Timing vs Attribute Menu Self-Init

**HUD Overlay Globes** (Traditional):
- Initialized by ATDHUD::InitializeHUD() 
- Controller created and assigned during HUD setup
- Immediate BroadcastInitialValues call
- Widget displays data right away

**Attribute Menu Self-Init** (New Pattern):
- Widget initializes itself in Event Construct
- May execute before or after HUD initialization
- Controller may not exist yet when widget constructs
- Requires defensive programming

### Defensive Timing Strategies

1. **Null Check Results**: Always check if controller is nullptr
2. **Retry Logic**: Consider using Timer to retry if controller unavailable
3. **Event-Driven**: Listen for controller availability events
4. **Lazy Initialization**: Initialize on first user interaction instead

### Example: Defensive Blueprint Pattern

```
Event Construct
    ↓
Get Attribute Menu Widget Controller
    ↓
Branch (Is Valid?)
    ├── True: Set Widget Controller → Broadcast Initial Values
    └── False: Set Timer (0.1s, Looping) → Retry Logic
```

## Troubleshooting

### Common Issues and Solutions

**Controller Returns Null**
- ✅ Check if PlayerController is valid
- ✅ Verify PlayerState exists
- ✅ Ensure ASC is properly initialized
- ✅ Confirm AttributeSet is created and added to ASC
- ✅ Validate HUD class is ATDHUD (not base AHUD)

**Multiple Controller Instances Created**
- ✅ Ensure ATDHUD caching logic is correct
- ✅ Check for race conditions in controller creation
- ✅ Verify controller class is set in HUD properties

**Widget Not Updating**
- ✅ Confirm controller delegates are properly bound
- ✅ Check if BroadcastInitialValues was called
- ✅ Verify widget delegates are bound to controller events
- ✅ Test attribute changes manually to verify delegate flow

**WorldContext Resolution Fails**
- ✅ Pass correct object reference (usually 'Self')
- ✅ Ensure calling context has valid world
- ✅ Check PlayerController index (usually 0 for single player)

### Debug Checklist

When implementing or debugging:

- [ ] Blueprint Library functions are marked `BlueprintCallable`
- [ ] WorldContext meta tag is properly set
- [ ] All pointer validations are in place
- [ ] HUD controller methods exist (GetHUDWidgetController, GetAttributeMenuWidgetController)
- [ ] Controller caching logic prevents duplicate creation
- [ ] Widget properly binds to controller delegates
- [ ] BroadcastInitialValues is called after widget controller assignment

### Testing Your Implementation

1. **Create Test Widget**: Simple widget that calls Blueprint Library function
2. **Log Results**: Add logging to see if controller is retrieved successfully  
3. **Verify Singleton**: Check that multiple calls return same controller instance
4. **Test Timing**: Try calling from different widget lifecycle events
5. **Validate Updates**: Modify attributes and confirm widget updates correctly

## Integration Notes

### Compatibility with Existing Systems

- **Existing HUD Overlay**: Continues to work normally via InitializeHUD()
- **Manual Controller Creation**: Can coexist with Blueprint Library approach
- **Multiple Widgets**: Multiple widgets can safely access same controller
- **Network Play**: Works in multiplayer (each player gets own controller)

### Performance Considerations

- **Lightweight Calls**: Blueprint Library functions are fast lookup operations
- **No Duplicate Bindings**: Singleton pattern prevents duplicate delegate subscriptions
- **Memory Efficient**: Single controller per player regardless of widget count
- **Blueprint Friendly**: Functions are optimized for Blueprint usage

## Related Documentation

- [Attribute Menu Widget Controller Setup](../attribute-menu/attribute-menu-widget-controller-setup.md#self-initializing-via-blueprint-library) - Self-initialization workflow
- [Widget Controllers Singletons FAQ](../../faq/widget-controllers-singletons.md) - Common questions and patterns
- [UI Widget Controller](../ui-widget-controller.md) - Base controller patterns and lifecycle