# Attribute Menu Node Usage (Blueprint Function Library API)

Last updated: 2024-12-19

## Overview

This guide provides API reference and usage examples for the two primary Blueprint Function Library nodes: **Get HUD Widget Controller** and **Get Attribute Menu Widget Controller**. These nodes enable widgets to access singleton-like controllers from any context using WorldContext resolution.

## API Reference

### Get HUD Widget Controller

**Function Signature**:
```cpp
UFUNCTION(BlueprintCallable, Category = "TD Widget Library",
          CallInEditor = true, meta = (WorldContext = "WorldContext"))
static UTDHUDWidgetController* GetHUDWidgetController(const UObject* WorldContext);
```

**Blueprint Node**:
- **Name**: Get HUD Widget Controller
- **Category**: TD Widget Library
- **Input**: World Context (Object Reference)  
- **Output**: TD HUD Widget Controller (Object Reference, nullable)

**Purpose**: Retrieves the cached HUD/Overlay Widget Controller used for primary player UI elements like health bars, mana bars, and experience displays.

**Usage Example**:
```
[Event Construct]
        ↓
[Get HUD Widget Controller] (World Context: Self)
        ↓
[Set Widget Controller] → [Broadcast Initial Values]
```

### Get Attribute Menu Widget Controller

**Function Signature**:
```cpp
UFUNCTION(BlueprintCallable, Category = "TD Widget Library", 
          CallInEditor = true, meta = (WorldContext = "WorldContext"))
static UTDAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);
```

**Blueprint Node**:
- **Name**: Get Attribute Menu Widget Controller
- **Category**: TD Widget Library  
- **Input**: World Context (Object Reference)
- **Output**: TD Attribute Menu Widget Controller (Object Reference, nullable)

**Purpose**: Retrieves the cached Attribute Menu Widget Controller used for character stat displays, attribute points allocation, and attribute-based UI elements.

**Usage Example**:
```
[Event Begin Play]
        ↓
[Get Attribute Menu Widget Controller] (World Context: Self)
        ↓
[Is Valid] ───True──→ [Set Widget Controller] → [Show Attributes]
    │
    └─False──→ [Set Timer] → [Retry Logic]
```

## WorldContext Parameter Explained

### What is WorldContext?

The WorldContext parameter is UE5's mechanism for resolving which world (game instance) and player the function should operate on. This is critical for multiplayer scenarios and editor testing.

**Technical Details**:
- **Type**: `const UObject*` - Any UObject that has world context
- **Purpose**: Enables resolution of UWorld → GameInstance → PlayerController → PlayerState
- **Meta Tag**: `meta = (WorldContext = "WorldContext")` marks parameter for UE5's context system

### Common WorldContext Values

| Context Object | Use Case | Availability |
|---|---|---|
| **Widget Self** | Most common - widget calling the function | Always available in widget context |
| **PlayerController** | Direct controller reference | Available if you have PC reference |
| **Pawn** | Character/pawn context | Available in pawn-based contexts |  
| **GameMode** | Server-side operations | Server only, not recommended for UI |
| **World** | Direct world reference | Rare, usually not needed for UI |

### Blueprint Usage Patterns

**Standard Pattern (Widget Self)**:
```
[Widget Event] → [Get Controller Node] (World Context: Self)
```

**Alternative Pattern (Custom Context)**:
```
[Custom Event] → [Get Controller Node] (World Context: Player Controller Reference)
```

**Editor Testing Pattern**:
```
[Editor Utility Widget] → [Get Controller Node] (World Context: Editor World Context)
```

## Multiplayer Considerations

### Client-Local Behavior

Both controller functions operate on **client-local** data:

- **Server Authority**: Attribute values are authoritative on server
- **Client Replication**: ASC replicates attribute changes to each client
- **Local Controllers**: Each client has its own HUD and controller instances
- **No Cross-Client Access**: Client A cannot access Client B's controllers

### Network Flow Example

```
Server: Health attribute changes (authoritative)
    ↓ [Network Replication]
Client 1 ASC: Receives health update  
    ↓ [Local Delegate]
Client 1 Controller: Processes health change
    ↓ [UI Broadcast]
Client 1 Widgets: Update health display

Client 2 ASC: Receives same health update
    ↓ [Local Delegate]  
Client 2 Controller: Processes health change
    ↓ [UI Broadcast]
Client 2 Widgets: Update health display
```

### Multiplayer Guidelines

**Call on Local Client Only**:
- Use these functions in widgets displayed to the local player
- Don't attempt to access other players' controllers
- Ensure ASC/AttributeSet replication is properly configured

**Network Safety**:
- Controllers are not replicated objects
- Controller data reflects local client's view of replicated data
- UI updates reflect server-authoritative attribute values after replication

## Implementation Requirements

### HUD Class Setup

The functions require specific HUD class configuration:

```cpp
// GameMode must use the correct HUD class
UCLASS()
class ATDGameMode : public AGameModeBase
{
public:
    ATDGameMode()
    {
        // Set HUD class to your project's HUD implementation
        HUDClass = ATDHUD::StaticClass(); // Not base AHUD class
    }
};
```

### Controller Class Assignment

In your HUD Blueprint or C++ constructor:

```cpp
// ATDHUD constructor or Blueprint defaults
HUDWidgetControllerClass = UTDHUDWidgetController::StaticClass();
AttributeMenuWidgetControllerClass = UTDAttributeMenuWidgetController::StaticClass();
```

### ASC/AttributeSet Requirements

Both functions require properly initialized gameplay systems:

**AbilitySystemComponent**:
- Must be initialized via `InitAbilityActorInfo()`
- Typically attached to PlayerState or Pawn
- Must implement `IAbilitySystemInterface`

**AttributeSet**:
- Must be added to ASC via `AddAttributeSet()` or `GetMutableAttributeSet()`
- Must be the correct project-specific type (e.g., `UTDAttributeSet`)
- Must have proper attribute replication setup

## Comprehensive Troubleshooting

### Function Returns Null

**Symptom**: Blueprint Library function returns null/invalid reference

**Diagnostic Steps**:

1. **Check PlayerController Resolution**:
   ```cpp
   // Add to Blueprint Library implementation
   if (!PC) 
   {
       UE_LOG(LogTemp, Warning, TEXT("PlayerController not found from WorldContext"));
       return nullptr;
   }
   ```

2. **Verify PlayerState Availability**:
   ```cpp  
   APlayerState* PS = PC->GetPlayerState();
   if (!PS)
   {
       UE_LOG(LogTemp, Warning, TEXT("PlayerState not initialized yet"));
       return nullptr;
   }
   ```

3. **Confirm ASC Resolution**:
   ```cpp
   if (!ASC)
   {
       UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent not found on PlayerState or Pawn"));
       return nullptr;
   }
   ```

4. **Validate AttributeSet Type**:
   ```cpp
   UAttributeSet* AttributeSet = ASC->GetAttributeSet(UTDAttributeSet::StaticClass());
   if (!AttributeSet)
   {
       UE_LOG(LogTemp, Warning, TEXT("UTDAttributeSet not found in ASC"));
       return nullptr;
   }
   ```

### HUD Class Issues

**Symptom**: HUD cast fails, controller classes not set

**Common Causes**:
- GameMode using base `AHUD` instead of `ATDHUD`
- HUD class not assigned in GameMode Blueprint
- Wrong HUD class inheritance hierarchy

**Solutions**:
```cpp
// Check HUD class in GameMode
if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
{
    // Success
}
else
{
    UE_LOG(LogTemp, Error, TEXT("HUD is not ATDHUD class: %s"), 
           PC->GetHUD() ? *PC->GetHUD()->GetClass()->GetName() : TEXT("Null"));
}
```

### Controller Class Not Set

**Symptom**: Controller instance is null even with valid HUD

**Diagnostic**:
```cpp
// In ATDHUD::GetAttributeMenuWidgetController()
if (!AttributeMenuWidgetControllerClass)
{
    UE_LOG(LogTemp, Error, TEXT("AttributeMenuWidgetControllerClass not set in HUD"));
    return nullptr;
}
```

**Solution**: Set controller class in HUD Blueprint defaults or C++ constructor

### Timing Issues

**Symptom**: Function works sometimes, fails other times

**Common Scenarios**:

1. **Early Widget Construction**:
   - Widget constructs before player possession complete
   - ASC not fully initialized when widget creates
   - PlayerState not replicated to client yet

2. **Level Transition**:  
   - HUD destroyed/recreated during level change
   - Controller references become stale
   - ASC re-initialization needed

**Mitigation Strategies**:

```cpp
// Blueprint Pattern: Retry with Timer
[Get Controller] → [Is Valid?] 
    ├─True──→ [Use Controller]
    └─False─→ [Set Timer: 0.1s] → [Retry Get Controller]
```

```cpp
// C++: Event-driven approach
UCLASS()
class UTDAttributeMenuWidget : public UUserWidget
{
    UFUNCTION()
    void OnPlayerStateReady();
    
    virtual void NativeConstruct() override
    {
        // Bind to possession/initialization events instead of immediate call
        GetWorld()->GetFirstPlayerController()->OnPossessedPawnChanged.AddUObject(this, &UTDAttributeMenuWidget::OnPlayerStateReady);
    }
};
```

### ASC Interface Issues

**Symptom**: ASC resolution fails even with valid PlayerState/Pawn

**Check Interface Implementation**:
```cpp
// PlayerState must implement IAbilitySystemInterface
UCLASS()
class ATDPlayerState : public APlayerState, public IAbilitySystemInterface
{
    // Must override GetAbilitySystemComponent()
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
```

**Alternative Pawn Implementation**:
```cpp
// If ASC is on Pawn instead of PlayerState
UCLASS()
class ATDCharacter : public ACharacter, public IAbilitySystemInterface  
{
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
```

### AttributeSet Type Mismatch

**Symptom**: ASC exists but AttributeSet not found

**Verify AttributeSet Addition**:
```cpp
// In PlayerState or Pawn initialization
void ATDPlayerState::BeginPlay()
{
    Super::BeginPlay();
    
    if (AbilitySystemComponent)
    {
        // Add AttributeSet to ASC
        AttributeSet = CreateDefaultSubobject<UTDAttributeSet>(TEXT("AttributeSet"));
        AbilitySystemComponent->SetAttributeSet(AttributeSet);
        
        // Or use AddAttributeSet for runtime addition
        AbilitySystemComponent->AddSpawnedAttribute(AttributeSet);
    }
}
```

**Check AttributeSet Type**:
```cpp
// Ensure correct StaticClass() call
UAttributeSet* AttributeSet = ASC->GetAttributeSet(UTDAttributeSet::StaticClass()); // Not base UAttributeSet
```

## Testing and Validation

### Blueprint Testing Approach

1. **Create Test Widget**:
   ```
   [Event Construct]
           ↓
   [Get Attribute Menu Widget Controller]
           ↓
   [Print String] (Text: "Controller Valid: {IsValid}")
           ↓
   [Branch: Is Valid?]
       ├─True──→ [Print String: "SUCCESS: Controller Retrieved"]
       └─False─→ [Print String: "ERROR: Controller is Null"]
   ```

2. **Test Both Functions**:
   - Create separate test cases for HUD and Attribute Menu controllers
   - Verify both return same singleton instance on multiple calls
   - Test from different widget contexts (Event Construct, Event Tick, Custom Events)

3. **Stress Test Timing**:
   - Call functions immediately on level load
   - Call after delays (0.1s, 0.5s, 1.0s)
   - Call from different lifecycle events

### C++ Testing Patterns

```cpp
// Unit test example
UCLASS()
class UTDWidgetTestSuite : public UObject
{
public:
    UFUNCTION(CallInEditor = true)
    void TestControllerRetrieval()
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        
        UTDAttributeMenuWidgetController* Controller1 = UTDWidgetBlueprintLibrary::GetAttributeMenuWidgetController(World);
        UTDAttributeMenuWidgetController* Controller2 = UTDWidgetBlueprintLibrary::GetAttributeMenuWidgetController(World);
        
        // Test singleton behavior
        check(Controller1 == Controller2);
        
        UE_LOG(LogTemp, Log, TEXT("Controller test passed: %s"), Controller1 ? TEXT("Valid") : TEXT("Null"));
    }
};
```

### Debug Console Commands

Add these to your project for debugging:

```cpp
// Custom console commands for debugging
UCLASS()
class UTDDebugCommands : public UObject
{
public:
    UFUNCTION(Exec)
    void DebugControllers()
    {
        if (UWorld* World = GetWorld())
        {
            APlayerController* PC = World->GetFirstPlayerController();
            if (PC && PC->GetHUD())
            {
                UE_LOG(LogTemp, Warning, TEXT("HUD Class: %s"), *PC->GetHUD()->GetClass()->GetName());
                UE_LOG(LogTemp, Warning, TEXT("PlayerState: %s"), PC->GetPlayerState() ? TEXT("Valid") : TEXT("Null"));
            }
        }
    }
};
```

## Best Practices

### Error Handling

**Always Check Validity**:
```
[Get Controller] → [Is Valid] → [Use if Valid, Handle if Null]
```

**Graceful Degradation**:
```
Null Controller → Show Loading State or Retry → Don't Crash Widget
```

**User Feedback**:
```  
Loading State → "Initializing player data..." → Actual Content
```

### Performance Optimization

**Cache Controller References**:
```cpp
// In widget class - cache after first successful retrieval
UPROPERTY()
UTDAttributeMenuWidgetController* CachedController;
```

**Avoid Repeated Calls**:
```
Call Once in Event Construct → Cache Result → Use Cached Reference
```

**Minimize WorldContext Resolution**:
- Pass `Self` as WorldContext (most efficient)
- Avoid complex context object chains
- Don't call functions in Tick events

### Blueprint Organization

**Consistent Naming**:
- Use same category for all widget library functions
- Follow project naming conventions (TD, Aura, etc.)
- Group related functions logically

**Clear Error Messages**:
```
Print String: "Attribute Menu Controller not available - check GameMode HUD class"
Print String: "Player not fully initialized - retrying in 0.1 seconds"  
```

**Modular Design**:
- Separate initialization logic into functions
- Use Blueprint Function Libraries for reusable patterns
- Create base widget classes with common controller logic

## Related Documentation

- [Attribute Menu Controller Guide](../attribute-menu/attribute-menu-controller-guide.md) - Implementation patterns and setup
- [Widget Controller Access Guide](./widget-controller-access-guide.md) - General Blueprint Function Library patterns
- [Widget Controllers Singletons FAQ](../../faq/widget-controllers-singletons.md) - Common questions and advanced scenarios
- [Attribute Menu Widget Controller Setup](../attribute-menu/attribute-menu-widget-controller-setup.md) - Complete setup guide