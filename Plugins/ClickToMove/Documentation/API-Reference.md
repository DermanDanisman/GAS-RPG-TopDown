# ClickToMove Plugin - API Reference

Complete API documentation for the ClickToMove plugin, including all public methods, properties, and events.

## 📚 Table of Contents

- [UClickToMoveComponent](#uclicktomovecomponent)
  - [Public Methods](#public-methods)
  - [Configuration Properties](#configuration-properties)
  - [Runtime State Properties](#runtime-state-properties)
  - [Debug Properties](#debug-properties)
- [Integration Examples](#integration-examples)
- [Blueprint Integration](#blueprint-integration)
- [C++ Integration](#c-integration)

## UClickToMoveComponent

Main component class that handles all click-to-move functionality. Inherits from `UActorComponent`.

### Class Declaration

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CLICKTOMOVE_API UClickToMoveComponent : public UActorComponent
```

---

## Public Methods

### Input Handling Methods

#### OnClickPressed()

Handles the start of a mouse click (LMB press).

```cpp
UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
void OnClickPressed();
```

**Description:**
- Stops any current autorun movement
- Resets press duration timer
- Clears any previously cached path
- Disables component ticking for performance

**Usage:**
```cpp
// C++ example
ClickToMoveComponent->OnClickPressed();
```

**Blueprint:** Available as BlueprintCallable function

---

#### OnClickHeld()

Handles continuous mouse button hold (called every frame while held).

```cpp
UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
void OnClickHeld(bool bUseInternalHitResult, const FHitResult& InHitResult);
```

**Parameters:**
- `bUseInternalHitResult` - If true, performs internal cursor trace on NAVIGATION channel
- `InHitResult` - External hit result to use when bUseInternalHitResult is false

**Description:**
- Accumulates hold time for press classification
- Performs cursor tracing (internal or external)
- Projects hit point to navigable surface
- Drives immediate character movement toward cursor

**Usage:**
```cpp
// C++ example - Internal trace
ClickToMoveComponent->OnClickHeld(true, FHitResult());

// C++ example - External hit result
FHitResult CustomHit = GetCustomHitResult();
ClickToMoveComponent->OnClickHeld(false, CustomHit);
```

**Blueprint:** Available as BlueprintCallable function

---

#### OnClickReleased()

Handles mouse button release (end of click).

```cpp
UFUNCTION(BlueprintCallable, Category="ClickToMove|Input")
void OnClickReleased();
```

**Description:**
- Evaluates press duration against ShortPressThreshold
- Builds navigation path for short presses
- Starts autorun if valid path found
- Ignores long presses (hold-to-move already handled movement)

**Usage:**
```cpp
// C++ example
ClickToMoveComponent->OnClickReleased();
```

**Blueprint:** Available as BlueprintCallable function

---

### Control Methods

#### SetIsTargeting()

Controls whether the component should yield to targeting/interaction systems.

```cpp
UFUNCTION(BlueprintCallable, Category="ClickToMove")
void SetIsTargeting(const bool bInTargeting);
```

**Parameters:**
- `bInTargeting` - True to disable movement (targeting mode), false to enable

**Description:**
- When true, OnClickHeld() returns early without processing movement
- Allows LMB to be shared between movement and interaction systems
- Automatically reset to false after movement commands

**Usage:**
```cpp
// C++ example - Disable movement during ability targeting
ClickToMoveComponent->SetIsTargeting(true);

// Enable movement after targeting complete
ClickToMoveComponent->SetIsTargeting(false);
```

**Blueprint:** Available as BlueprintCallable function

---

#### SetAutoRunActive()

Directly controls autorun state (normally managed internally).

```cpp
UFUNCTION(BlueprintCallable, Category="ClickToMove")
void SetAutoRunActive(const bool bInActive);
```

**Parameters:**
- `bInActive` - True to enable autorun, false to disable

**Description:**
- External control over autorun state
- Useful for higher-level systems to cancel or force movement
- Does not clear path data (use StopMovement() for complete reset)

**Usage:**
```cpp
// C++ example - Force stop autorun
ClickToMoveComponent->SetAutoRunActive(false);
```

**Blueprint:** Available as BlueprintCallable function

---

#### StopMovement()

Completely stops all movement and clears state.

```cpp
UFUNCTION(BlueprintCallable, Category="ClickToMove|Orders")
void StopMovement();
```

**Description:**
- Disables autorun and component ticking
- Clears cached path points
- Resets path index
- Use when you need to completely cancel movement

**Usage:**
```cpp
// C++ example - Emergency movement stop
ClickToMoveComponent->StopMovement();
```

**Blueprint:** Available as BlueprintCallable function

---

### Getter Methods

These methods are available for reading component state:

```cpp
// Get current cached destination
FVector GetCachedDestination() const { return CachedDestination; }

// Get current follow time
float GetFollowTime() const { return FollowTime; }

// Check if autorun is active
bool IsAutoRunning() const { return bIsAutoRunning; }

// Check if in targeting mode
bool IsTargeting() const { return bIsTargeting; }

// Get current path points
const TArray<FVector>& GetPathPoints() const { return PathPoints; }

// Get current path index
int32 GetPathIndex() const { return PathIndex; }
```

---

## Configuration Properties

### Movement Timing

#### ShortPressThreshold

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.05"))
float ShortPressThreshold = 0.5f;
```

**Description:** Maximum duration (seconds) for a click to trigger autorun
**Default:** 0.5 seconds
**Range:** 0.05+ seconds

---

### Acceptance Radius Settings

#### AcceptanceRadius

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="1.0"))
float AcceptanceRadius = 50.f;
```

**Description:** Base distance to consider arrival at target
**Default:** 50 units
**Range:** 1.0+ units

#### bScaleAcceptanceBySpeed

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
bool bScaleAcceptanceBySpeed = true;
```

**Description:** Whether to scale acceptance radius based on movement speed
**Default:** true

#### AcceptanceSpeedScale

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0"))
float AcceptanceSpeedScale = 0.05f;
```

**Description:** Speed scaling factor (radius += speed * scale)
**Default:** 0.05
**Range:** 0.0+

#### AcceptanceRadiusMin / AcceptanceRadiusMax

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0"))
float AcceptanceRadiusMin = 30.f;

UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0"))
float AcceptanceRadiusMax = 120.f;
```

**Description:** Bounds for speed-scaled acceptance radius
**Defaults:** 30 / 120 units

---

### Lookahead Settings

#### bUseLookahead

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
bool bUseLookahead = true;
```

**Description:** Enable corner-cutting by blending toward next path point
**Default:** true

#### LookaheadBlendAlpha

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config", meta=(ClampMin="0.0", ClampMax="1.0"))
float LookaheadBlendAlpha = 0.3f;
```

**Description:** Blend strength between current and next target (0=no lookahead, 1=full lookahead)
**Default:** 0.3
**Range:** 0.0 - 1.0

---

### Navigation Settings

#### CursorTraceChannel

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
TEnumAsByte<ECollisionChannel> CursorTraceChannel = NAVIGATION;
```

**Description:** Collision channel for internal cursor traces
**Default:** NAVIGATION (defined in ClickToMove.h)

#### NavProjectExtent

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Config")
FVector NavProjectExtent = FVector(200.f, 200.f, 200.f);
```

**Description:** Half-extents for navmesh projection search box
**Default:** (200, 200, 200)

---

## Runtime State Properties

These properties are visible for debugging but should not be modified directly:

### CachedDestination

```cpp
UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
FVector CachedDestination = FVector::ZeroVector;
```

**Description:** Current or last known destination point

### FollowTime

```cpp
UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State", meta=(ClampMin="0.0"))
float FollowTime = 0.f;
```

**Description:** Accumulated hold time for current press

### bIsAutoRunning

```cpp
UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
bool bIsAutoRunning = false;
```

**Description:** Whether autorun path following is active

### bIsTargeting

```cpp
UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
bool bIsTargeting = false;
```

**Description:** Whether movement is disabled for targeting/interaction

### PathPoints

```cpp
UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
TArray<FVector> PathPoints;
```

**Description:** Cached navigation path points

### PathIndex

```cpp
UPROPERTY(VisibleInstanceOnly, Category="ClickToMove|State")
int32 PathIndex = INDEX_NONE;
```

**Description:** Current target index in PathPoints array

---

## Debug Properties

### bDebugProjectToNav

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Debug")
bool bDebugProjectToNav = false;
```

**Description:** Enable debug visualization for navmesh projection

### Debug Colors and Timing

```cpp
UPROPERTY(EditAnywhere, Category="ClickToMove|Debug")
FColor DebugProjectBoxColor = FColor::Cyan;

UPROPERTY(EditAnywhere, Category="ClickToMove|Debug")
FColor DebugProjectedPointColor = FColor::Green;

UPROPERTY(EditAnywhere, Category="ClickToMove|Debug", meta=(ClampMin="0.0"))
float DebugDrawLifetime = 1.5f;

UPROPERTY(EditAnywhere, Category="ClickToMove|Debug", meta=(ClampMin="0.1"))
float DebugLineThickness = 1.5f;
```

---

## Integration Examples

### Basic PlayerController Integration

```cpp
// Header file
UCLASS()
class YOURGAME_API AYourPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AYourPlayerController();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UClickToMoveComponent> ClickToMoveComponent;

    void OnClickPressed();
    void OnClickHeld();
    void OnClickReleased();
};

// Implementation
AYourPlayerController::AYourPlayerController()
{
    ClickToMoveComponent = CreateDefaultSubobject<UClickToMoveComponent>(TEXT("ClickToMoveComponent"));
}
```

### Enhanced Input Integration

```cpp
void AYourPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &AYourPlayerController::OnClickPressed);
        EIC->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AYourPlayerController::OnClickHeld);
        EIC->BindAction(ClickAction, ETriggerEvent::Completed, this, &AYourPlayerController::OnClickReleased);
    }
}
```

### GAS Integration Example

```cpp
void AYourPlayerController::OnClickHeld()
{
    // Priority: Ability targeting > Movement
    if (IsTargetingAbility())
    {
        ClickToMoveComponent->SetIsTargeting(true);
        ForwardToAbilitySystem();
    }
    else
    {
        ClickToMoveComponent->SetIsTargeting(false);
        ClickToMoveComponent->OnClickHeld(true, FHitResult());
    }
}
```

### Custom Interaction System Integration

```cpp
void AYourPlayerController::OnClickHeld()
{
    if (InteractionSystem->HasValidTarget())
    {
        // Use hit result from interaction system
        FHitResult InteractionHit = InteractionSystem->GetCurrentHitResult();
        ClickToMoveComponent->OnClickHeld(false, InteractionHit);
    }
    else
    {
        // Use internal navigation trace
        ClickToMoveComponent->OnClickHeld(true, FHitResult());
    }
}
```

---

## Blueprint Integration

### Blueprint Events

All public methods are available as Blueprint callable functions:

- **OnClickPressed** - Call on Enhanced Input Started event
- **OnClickHeld** - Call on Enhanced Input Triggered event  
- **OnClickReleased** - Call on Enhanced Input Completed event

### Blueprint Property Access

Configuration properties are editable in Blueprint defaults:

1. Select ClickToMoveComponent in Blueprint
2. Configure properties in Details panel under **ClickToMove|Config**
3. Runtime state visible under **ClickToMove|State**
4. Debug options under **ClickToMove|Debug**

### Blueprint Function Examples

```blueprint
// Basic input handling
Event BeginPlay
├── Enhanced Input Component → Bind Action (ClickAction, Started, OnClickPressed)
├── Enhanced Input Component → Bind Action (ClickAction, Triggered, OnClickHeld)  
└── Enhanced Input Component → Bind Action (ClickAction, Completed, OnClickReleased)

Event OnClickPressed
└── ClickToMoveComponent → OnClickPressed

Event OnClickHeld  
├── ClickToMoveComponent → OnClickHeld
├── bUseInternalHitResult: True
└── InHitResult: (empty)

Event OnClickReleased
└── ClickToMoveComponent → OnClickReleased
```

---

## C++ Integration

### Including Headers

```cpp
#include "Components/ClickToMoveComponent.h"
```

### Component Creation

```cpp
// In constructor
ClickToMoveComponent = CreateDefaultSubobject<UClickToMoveComponent>(TEXT("ClickToMoveComponent"));
```

### Runtime Configuration

```cpp
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (ClickToMoveComponent)
    {
        // Customize behavior
        ClickToMoveComponent->ShortPressThreshold = 0.3f;
        ClickToMoveComponent->AcceptanceRadius = 25.0f;
        ClickToMoveComponent->bUseLookahead = true;
        ClickToMoveComponent->LookaheadBlendAlpha = 0.4f;
    }
}
```

### State Monitoring

```cpp
void AYourPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (ClickToMoveComponent)
    {
        // Monitor component state
        bool bMoving = ClickToMoveComponent->IsAutoRunning();
        FVector Destination = ClickToMoveComponent->GetCachedDestination();
        
        // Update UI or other systems based on movement state
        UpdateMovementUI(bMoving, Destination);
    }
}
```

---

## Performance Notes

### Optimization Features

- **Conditional Ticking**: Component only ticks during autorun
- **Local Controller Validation**: Prevents unnecessary execution on server/remote clients
- **Debug Exclusion**: All debug drawing excluded from shipping builds
- **Efficient Path Caching**: Reuses computed navigation paths

### Memory Usage

- **Minimal Static Memory**: ~1KB per component when idle
- **Dynamic Memory**: Path arrays scale with navigation complexity
- **No Persistent Objects**: Spline component unregistered (no scene cost)

### Performance Tips

```cpp
// Recommended settings for performance
ClickToMoveComponent->AcceptanceRadius = 50.0f;           // Balance precision vs oscillation
ClickToMoveComponent->bScaleAcceptanceBySpeed = true;     // Reduce speed-related issues
ClickToMoveComponent->NavProjectExtent = FVector(200.0f); // Reasonable search volume
ClickToMoveComponent->bDebugProjectToNav = false;         // Disable in shipping
```

---

This API reference provides complete documentation for integrating and using the ClickToMove plugin in your Unreal Engine 5 project. For setup instructions, see the [Setup Guide](Setup-Guide.md).