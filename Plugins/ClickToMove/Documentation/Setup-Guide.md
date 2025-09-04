# ClickToMove Plugin - Setup Guide

This guide walks you through setting up the ClickToMove plugin in your Unreal Engine 5 project, from initial installation to advanced integration scenarios.

## 📋 Prerequisites

- **Unreal Engine 5.0 or later**
- **Enhanced Input System** (recommended)
- **Navigation System** enabled in project
- **C++ project** (optional, for advanced integration)

## 🔧 Installation

### Step 1: Plugin Installation

1. **Download and Extract**
   - Copy the `ClickToMove` folder to your project's `Plugins` directory
   - If `Plugins` folder doesn't exist, create it in your project root

2. **Enable Plugin**
   - Open your project in Unreal Editor
   - Go to **Edit → Plugins**
   - Search for "ClickToMove" in the **Gameplay** category
   - Check the **Enabled** checkbox
   - Restart Unreal Editor when prompted

3. **Verify Installation**
   - In the Content Browser, enable "Show Plugin Content"
   - You should see "ClickToMove Content" folder (if plugin has content)
   - Check that `UClickToMoveComponent` is available in Blueprint creation menus

### Step 2: Project Configuration

#### Configure Collision Channels

The plugin uses a custom collision channel for cursor tracing:

1. **Open Project Settings**
   - Go to **Edit → Project Settings**
   - Navigate to **Engine → Collision**

2. **Add Collision Channel**
   - Click **New...** under **Collision Profiles**
   - Name: `NAVIGATION`
   - Default Response: `Block`
   - Description: "Navigation trace channel for click-to-move"

3. **Configure Object Responses**
   - Set your ground/floor objects to **Block** the NAVIGATION channel
   - Set your character pawns to **Ignore** the NAVIGATION channel
   - Set interactive objects based on your needs (typically **Ignore**)

#### Enable Navigation System

1. **Navigation Settings**
   - In Project Settings, go to **Engine → Navigation System**
   - Ensure **Mode** is set to "Automatic" or "Custom"
   - Check **Generate Navigation Data** is enabled

2. **Add NavMesh to Level**
   - In your level, add a **NavMeshBoundsVolume**
   - Scale it to cover your playable area
   - Verify green navmesh appears in viewport

## 🎮 Basic Setup

### Blueprint-Only Integration

#### Step 1: Add Component to Player Controller

1. **Open Player Controller Blueprint**
   - Navigate to your custom PlayerController Blueprint
   - If you don't have one, create from **Blueprints → GameMode → Player Controller**

2. **Add ClickToMove Component**
   - In the Components panel, click **Add Component**
   - Search for "Click To Move Component"
   - Add and rename to "ClickToMoveComponent"

3. **Configure Component**
   - Select the component in the viewport
   - In the Details panel, configure under **ClickToMove|Config**:
     - **Short Press Threshold**: 0.5 (seconds)
     - **Acceptance Radius**: 50.0 (units)
     - **Cursor Trace Channel**: NAVIGATION

#### Step 2: Setup Input

1. **Create Input Action**
   - In Content Browser, create **Input → Input Action**
   - Name it "IA_ClickMove"
   - Set **Value Type** to "Digital (bool)"

2. **Setup Input Mapping Context**
   - Open your **Input Mapping Context** asset
   - Add mapping for "IA_ClickMove"
   - Bind to **Left Mouse Button**

3. **Bind Input in Player Controller**
   - In your PlayerController Blueprint's **BeginPlay**:
     - Get Enhanced Input Component
     - Bind Action "IA_ClickMove" to custom events

4. **Create Input Event Handlers**

```blueprint
// Event: On Click Pressed (ETriggerEvent::Started)
Event OnClickPressed
├── Call Function: ClickToMoveComponent → OnClickPressed

// Event: On Click Held (ETriggerEvent::Triggered)  
Event OnClickHeld
├── Call Function: ClickToMoveComponent → OnClickHeld
├── bUseInternalHitResult: True
├── InHitResult: (empty)

// Event: On Click Released (ETriggerEvent::Completed)
Event OnClickReleased
├── Call Function: ClickToMoveComponent → OnClickReleased
```

### C++ Integration

#### Step 1: Add Module Dependency

In your project's `.Build.cs` file:

```cpp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject", 
    "Engine",
    "ClickToMove"  // Add this line
});
```

#### Step 2: Include Headers

In your PlayerController header file:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/ClickToMoveComponent.h" // Add this include
#include "YourPlayerController.generated.h"

UCLASS()
class YOURGAME_API AYourPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AYourPlayerController();

protected:
    virtual void SetupInputComponent() override;
    
    // Input handlers
    void OnClickPressed();
    void OnClickHeld();
    void OnClickReleased();

    // Component reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UClickToMoveComponent> ClickToMoveComponent;

    // Input assets
    UPROPERTY(EditAnywhere, Category = "Enhanced Input")
    TObjectPtr<class UInputAction> ClickAction;
};
```

#### Step 3: Implementation

In your PlayerController source file:

```cpp
#include "YourPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AYourPlayerController::AYourPlayerController()
{
    // Create component
    ClickToMoveComponent = CreateDefaultSubobject<UClickToMoveComponent>(TEXT("ClickToMoveComponent"));
}

void AYourPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (ClickAction)
        {
            EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Started, this, &AYourPlayerController::OnClickPressed);
            EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AYourPlayerController::OnClickHeld);
            EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Completed, this, &AYourPlayerController::OnClickReleased);
        }
    }
}

void AYourPlayerController::OnClickPressed()
{
    if (ClickToMoveComponent)
    {
        ClickToMoveComponent->OnClickPressed();
    }
}

void AYourPlayerController::OnClickHeld()
{
    if (ClickToMoveComponent)
    {
        // Use internal trace to find ground under cursor
        ClickToMoveComponent->OnClickHeld(true, FHitResult());
    }
}

void AYourPlayerController::OnClickReleased()
{
    if (ClickToMoveComponent)
    {
        ClickToMoveComponent->OnClickReleased();
    }
}
```

## 🔧 Advanced Integration

### Gameplay Ability System Integration

If your project uses GAS, you can integrate click-to-move with ability targeting:

```cpp
// In your enhanced PlayerController
void AYourPlayerController::OnClickHeld()
{
    // Check if we're targeting an interactable actor
    if (HighlightInteraction->GetHighlightedActor())
    {
        // Forward to ability system for targeting
        if (UTDAbilitySystemComponent* ASC = GetASC())
        {
            ASC->AbilityInputTagHeld(FTDGameplayTags::Get().InputTag_LMB);
        }
    }
    else
    {
        // Use movement system
        if (ClickToMoveComponent)
        {
            ClickToMoveComponent->OnClickHeld(true, FHitResult());
        }
    }
}
```

### Custom Interaction System Integration

For projects with custom interaction/highlighting systems:

```cpp
void AYourPlayerController::OnClickHeld()
{
    if (ClickToMoveComponent)
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
}
```

### Multiplayer Considerations

The plugin is designed for multiplayer use:

```cpp
// Component automatically handles local controller validation
// No additional networking code required

// Optional: Sync visual state for other players
UFUNCTION(Server, Reliable)
void ServerNotifyMovementTarget(const FVector& TargetLocation);

void AYourPlayerController::OnClickReleased()
{
    if (ClickToMoveComponent)
    {
        ClickToMoveComponent->OnClickReleased();
        
        // Optional: Inform server of movement intention for other players
        if (IsLocalController())
        {
            ServerNotifyMovementTarget(ClickToMoveComponent->GetCachedDestination());
        }
    }
}
```

## 🔧 Configuration

### Component Settings

Configure the component behavior in Blueprint defaults or at runtime:

```cpp
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (ClickToMoveComponent)
    {
        // Customize behavior at runtime
        ClickToMoveComponent->SetShortPressThreshold(0.3f);    // Faster autorun trigger
        ClickToMoveComponent->SetAcceptanceRadius(25.0f);      // Tighter arrival detection
        ClickToMoveComponent->SetUseLookahead(true);           // Enable corner cutting
        ClickToMoveComponent->SetLookaheadBlendAlpha(0.4f);    // Stronger corner cutting
    }
}
```

### Debug Configuration

Enable debug visualization during development:

```cpp
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    #if UE_BUILD_DEVELOPMENT
    if (ClickToMoveComponent)
    {
        // Enable debug visualization
        ClickToMoveComponent->SetDebugProjectToNav(true);
        ClickToMoveComponent->SetDebugDrawLifetime(2.0f);
    }
    #endif
}
```

## ✅ Testing Your Setup

### Basic Functionality Test

1. **Test Hold-to-Move**
   - Hold left mouse button
   - Character should move toward cursor
   - Movement should stop when button released

2. **Test Short-Press Autorun**
   - Click briefly on distant location
   - Character should path to destination automatically
   - Should follow navmesh accurately around obstacles

3. **Test Navigation Projection**
   - Click on walls or non-walkable surfaces
   - Character should move to nearest walkable location

### Debug Verification

Enable debug mode and verify:

1. **Path Visualization**
   - Green spheres should appear at path points
   - Yellow sphere marks current target
   - Cyan sphere shows lookahead aim point

2. **Acceptance Radius**
   - Green circle around current target
   - Should scale with movement speed if enabled

3. **Navigation Projection**
   - Cyan box shows search area
   - Green sphere shows projected result

## 🐛 Common Setup Issues

### Issue: Character doesn't respond to clicks

**Solutions:**
- Verify input action is bound correctly
- Check collision channel configuration
- Ensure NavMesh exists in level
- Confirm component functions are being called

### Issue: Movement feels imprecise

**Solutions:**
- Reduce acceptance radius
- Disable speed-based scaling
- Adjust lookahead settings
- Check navmesh quality

### Issue: Multiplayer desync

**Solutions:**
- Verify only local controllers execute movement
- Check CharacterMovement replication settings
- Ensure consistent navmesh across clients

### Issue: Integration conflicts

**Solutions:**
- Review input binding order
- Check component priority settings
- Verify targeting mode cooperation
- Test interaction system integration

## 📈 Performance Optimization

### Recommended Settings

For optimal performance:

```cpp
// In component configuration
AcceptanceRadius = 50.0f;           // Balance precision vs performance
bScaleAcceptanceBySpeed = true;     // Reduce oscillation at speed
NavProjectExtent = FVector(200.0f); // Reasonable search volume
bUseLookahead = true;               // Smoother paths with minimal cost
```

### Profiling Tips

Monitor these metrics:
- Component tick frequency (should be 0 when idle)
- Navigation queries per frame
- Debug draw call count (development only)

## 🎯 Next Steps

After completing basic setup:

1. **Customize Movement Behavior** - Adjust parameters for your game feel
2. **Integrate with Game Systems** - Connect with abilities, interactions, UI
3. **Add Visual Polish** - Create movement indicators, path preview
4. **Test Multiplayer** - Verify behavior across network conditions
5. **Performance Profile** - Optimize for your target platform

For advanced customization and API reference, see the [API Documentation](API-Reference.md).