# ClickToMove Plugin for Unreal Engine 5

**A multiplayer-ready, high-performance click-to-move system with advanced pathfinding and dual movement modes.**

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.0+-blue?logo=unrealengine)
![License](https://img.shields.io/badge/License-Commercial-green)
![Version](https://img.shields.io/badge/Version-0.1.0-orange)

## 🎯 Overview

The ClickToMove plugin provides a professional-grade point-and-click movement system for top-down and isometric games. Designed with multiplayer networking, performance, and user experience in mind, it offers two distinct movement modes that can be seamlessly integrated into any Unreal Engine 5 project.

### 🌟 Key Features

- **Dual Movement Modes**
  - 🖱️ **Hold-to-Move**: Direct character steering while mouse button is held
  - 🎯 **Short-Press Autorun**: Click to set destination and automatically follow optimized path

- **🌐 Multiplayer Ready**
  - Client-side execution with server replication via CharacterMovement
  - Local controller validation prevents unauthorized movement
  - Network-optimized design minimizes bandwidth usage

- **🗺️ Advanced Navigation**
  - Seamless integration with UE5's Navigation System
  - Automatic navmesh projection for any surface clicks
  - Intelligent pathfinding with corner-cutting optimization
  - Speed-based acceptance radius scaling

- **⚡ Performance Optimized**
  - Conditional ticking (only active during autorun)
  - Efficient path caching and reuse
  - Minimal per-frame overhead when idle
  - Debug visualization excluded from shipping builds

- **🎛️ Highly Configurable**
  - Extensive Blueprint-exposed parameters
  - Runtime behavior modification
  - Debug visualization tools
  - Integration-friendly API design

- **🔗 Framework Integration**
  - Built for Enhanced Input System
  - Gameplay Ability System (GAS) compatible
  - Interaction system cooperation (targeting mode)
  - Modular component-based architecture

## 🚀 Quick Start

### Installation

1. Copy the `ClickToMove` folder to your project's `Plugins` directory
2. Enable the plugin in **Edit → Plugins → Gameplay → ClickToMove**
3. Restart Unreal Editor when prompted
4. Add module dependency (optional, for C++ projects):

```cpp
// In your .Build.cs file
PublicDependencyModuleNames.AddRange(new string[] {
    "ClickToMove"
});
```

### Basic Setup

1. **Add Component to Player Controller:**
```cpp
// In your PlayerController constructor
ClickToMoveComponent = CreateDefaultSubobject<UClickToMoveComponent>(TEXT("ClickToMoveComponent"));
```

2. **Blueprint Integration:**
   - Add `ClickToMoveComponent` to your PlayerController Blueprint
   - Configure input bindings to call component functions

3. **Input Binding Example:**
```cpp
// In your PlayerController's input setup
void AYourPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // Enhanced Input binding example
    if (ClickAction)
    {
        EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Started, 
            this, &AYourPlayerController::OnClickPressed);
        EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, 
            this, &AYourPlayerController::OnClickHeld);
        EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Completed, 
            this, &AYourPlayerController::OnClickReleased);
    }
}

void AYourPlayerController::OnClickPressed()
{
    ClickToMoveComponent->OnClickPressed();
}

void AYourPlayerController::OnClickHeld()
{
    ClickToMoveComponent->OnClickHeld(true, FHitResult());
}

void AYourPlayerController::OnClickReleased()
{
    ClickToMoveComponent->OnClickReleased();
}
```

## 📋 Component Overview

### UClickToMoveComponent

The core component that handles all click-to-move functionality. Designed as a lightweight ActorComponent that can be added to either PlayerControllers (recommended) or Pawns.

**Key Responsibilities:**
- Input processing and movement decision logic
- Navigation system integration and path building
- Character movement input generation
- Debug visualization and development tools

**Ownership Models:**
- **PlayerController** (Recommended): Centralized control, easy input handling
- **Pawn**: Direct attachment, useful for specialized character types

## 🎮 Movement Modes

### Hold-to-Move Mode

Direct character control while the mouse button is held down.

**Behavior:**
- Character moves toward cursor position in real-time
- Cursor hits are projected onto navigable surfaces
- Movement stops immediately when mouse button is released
- No path building or memory of destination

**Best For:**
- Precise character positioning
- Manual navigation around obstacles
- Real-time tactical movement

### Short-Press Autorun Mode

Automated path following after a quick click.

**Behavior:**
- Click duration under threshold triggers path building
- Character follows optimized navmesh path to destination
- Automatic corner-cutting with lookahead
- Continues until destination reached or interrupted

**Best For:**
- Long-distance travel
- AFK-friendly movement
- Smooth, predictable navigation

## ⚙️ Configuration Parameters

### Movement Tuning

| Parameter | Description | Default | Range |
|-----------|-------------|---------|--------|
| `ShortPressThreshold` | Max duration for autorun trigger | 0.5s | 0.05s+ |
| `AcceptanceRadius` | Distance to consider "arrived" | 50 units | 1.0+ |
| `bScaleAcceptanceBySpeed` | Dynamic radius based on velocity | true | - |
| `AcceptanceSpeedScale` | Speed scaling factor | 0.05 | 0.0+ |
| `AcceptanceRadiusMin/Max` | Scaled radius bounds | 30/120 | 0.0+ |

### Pathfinding

| Parameter | Description | Default |
|-----------|-------------|---------|
| `bUseLookahead` | Enable corner-cutting | true |
| `LookaheadBlendAlpha` | Lookahead strength | 0.3 |
| `NavProjectExtent` | Navmesh search volume | (200,200,200) |
| `CursorTraceChannel` | Collision channel for traces | NAVIGATION |

### Debug Options

| Parameter | Description | Default |
|-----------|-------------|---------|
| `bDebugProjectToNav` | Show navmesh projection | false |
| `DebugProjectBoxColor` | Search box color | Cyan |
| `DebugProjectedPointColor` | Result point color | Green |
| `DebugDrawLifetime` | Debug duration | 1.5s |

## 🔧 Advanced Integration

### Gameplay Ability System (GAS)

The plugin seamlessly integrates with GAS through targeting mode cooperation:

```cpp
// In your PlayerController
void AYourPlayerController::AbilityInputActionHeld(const FGameplayTag InputTag)
{
    if (InputTag.MatchesTagExact(FYourGameplayTags::Get().InputTag_LMB))
    {
        if (HighlightInteraction->GetHighlightedActor())
        {
            // Forward to ability system for targeting
            if (GetASC())
            {
                GetASC()->AbilityInputTagHeld(InputTag);
            }
        }
        else
        {
            // Use movement system
            ClickToMoveComponent->OnClickHeld(true, FHitResult());
        }
    }
}
```

### Custom Interaction Systems

Integration with highlight/interaction systems:

```cpp
// Use external hit results from your interaction system
void HandleInteractionInput(const FHitResult& InteractionHit)
{
    if (InteractionHit.bBlockingHit)
    {
        // Use hit from interaction system instead of internal trace
        ClickToMoveComponent->OnClickHeld(false, InteractionHit);
    }
}
```

### Runtime Control

```cpp
// Disable movement during cutscenes or special states
ClickToMoveComponent->SetIsTargeting(true);

// Force stop current movement
ClickToMoveComponent->StopMovement();

// Toggle autorun programmatically
ClickToMoveComponent->SetAutoRunActive(false);
```

## 🐛 Troubleshooting

### Common Issues

**Character doesn't move when clicking:**
- Verify component is added to PlayerController
- Check input bindings are correctly calling component functions
- Ensure NavMesh exists in the level
- Confirm collision channel setup (NAVIGATION channel should block ground)

**Movement feels sluggish or imprecise:**
- Reduce `AcceptanceRadius` for tighter movement
- Disable `bScaleAcceptanceBySpeed` for consistent behavior
- Adjust `LookaheadBlendAlpha` for sharper or smoother turns

**Multiplayer desync issues:**
- Verify only local controllers execute movement logic
- Check CharacterMovement replication settings
- Ensure consistent navmesh across server/clients

### Debug Tools

Enable debug visualization in development:

```cpp
// In component defaults or runtime
ClickToMoveComponent->bDebugProjectToNav = true;
```

Console commands:
```
showdebug ai          // Show navigation debug
showdebug navigation  // Show navmesh visualization
```

## 📈 Performance Considerations

### Optimization Features

- **Conditional Ticking**: Component only ticks during active autorun
- **Trace Minimization**: Reuses hit results when possible
- **Path Caching**: Stores computed paths for reuse
- **Debug Exclusion**: All debug code excluded from shipping builds

### Best Practices

1. **NavMesh Optimization**: Keep navmesh complexity reasonable for target platform
2. **Trace Channel Setup**: Use dedicated channel to avoid unnecessary collision checks
3. **Component Placement**: Prefer PlayerController ownership for better networking
4. **Input Throttling**: Consider input rate limiting for rapid clicking scenarios

## 🏗️ Architecture Notes

### Design Principles

- **Separation of Concerns**: Input handling, pathfinding, and movement are cleanly separated
- **Network Friendly**: Client prediction with server authority through CharacterMovement
- **Framework Agnostic**: Core functionality independent of specific input/ability systems
- **Performance First**: Designed for minimal overhead in production environments

### Extension Points

The plugin is designed for extension:

```cpp
// Inherit from UClickToMoveComponent for custom behavior
class YOURGAME_API UCustomClickToMoveComponent : public UClickToMoveComponent
{
    // Override specific behaviors while maintaining core functionality
    virtual void AutoRun() override;
    virtual void FindPathToLocation() override;
};
```

## 📞 Support and Contact

For technical support, bug reports, or feature requests:

- **GitHub Issues**: [Project Repository](https://github.com/DermanDanisman/GAS-RPG-TopDown)
- **Documentation**: See `/docs` folder for detailed guides
- **Email**: [Contact Developer]

## 📄 License

© 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.

Unreal Engine and its associated trademarks are used under license from Epic Games.

---

**Built with ❤️ for the Unreal Engine community**