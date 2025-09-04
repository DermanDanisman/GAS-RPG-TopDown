# ClickToMove Plugin - Configuration Guide

This guide covers all configuration options for the ClickToMove plugin, helping you fine-tune the movement behavior for your specific game needs.

## 📋 Configuration Overview

The ClickToMove plugin offers extensive configuration options organized into logical categories:

- **Movement Timing** - Controls press duration thresholds
- **Acceptance Radius** - Arrival detection and speed scaling
- **Pathfinding** - Navigation and lookahead behavior
- **Collision** - Trace channels and projection settings
- **Debug** - Development and visualization tools

All settings can be configured in Blueprint defaults or modified at runtime via C++.

## ⏱️ Movement Timing Configuration

### Short Press Threshold

**Property:** `ShortPressThreshold`  
**Type:** `float`  
**Default:** `0.5` seconds  
**Range:** `0.05+` seconds

Controls the maximum duration for a click to trigger autorun mode.

**Behavior:**
- **Short Press** (≤ threshold): Builds path and starts autorun
- **Long Press** (> threshold): Uses hold-to-move, no autorun

**Tuning Guidelines:**

```cpp
// Fast-paced action games
ShortPressThreshold = 0.2f;  // Quick autorun trigger

// Strategy/RPG games  
ShortPressThreshold = 0.5f;  // Balanced (default)

// Accessibility/casual games
ShortPressThreshold = 0.8f;  // More forgiving
```

**Blueprint Configuration:**
- Select ClickToMoveComponent
- Details Panel → ClickToMove|Config → Short Press Threshold

**Runtime Modification:**
```cpp
ClickToMoveComponent->ShortPressThreshold = 0.3f;
```

## 🎯 Acceptance Radius Configuration

Controls when the character considers they have "arrived" at a target location.

### Base Acceptance Radius

**Property:** `AcceptanceRadius`  
**Type:** `float`  
**Default:** `50.0` units  
**Range:** `1.0+` units

Base distance threshold for arrival detection.

**Tuning Guidelines:**

| Character Size | Recommended Radius | Use Case |
|----------------|-------------------|----------|
| Small (< 50 units) | 25-40 units | Precise movement |
| Medium (50-100 units) | 40-60 units | Balanced (default) |
| Large (100+ units) | 60-100 units | Smooth movement |

```cpp
// Precise movement for small characters
AcceptanceRadius = 30.0f;

// Standard movement for most characters
AcceptanceRadius = 50.0f;  // Default

// Smooth movement for large characters  
AcceptanceRadius = 80.0f;
```

### Speed-Based Scaling

Dynamic acceptance radius adjustment based on movement speed.

#### bScaleAcceptanceBySpeed

**Property:** `bScaleAcceptanceBySpeed`  
**Type:** `bool`  
**Default:** `true`

Enables dynamic radius scaling to prevent overshoot at high speeds.

**When Enabled:**
- Faster movement = larger acceptance radius
- Prevents oscillation around targets
- Smoother deceleration approach

**When Disabled:**
- Fixed radius regardless of speed
- More precise but may cause overshoot
- Useful for very controlled movement

#### AcceptanceSpeedScale

**Property:** `AcceptanceSpeedScale`  
**Type:** `float`  
**Default:** `0.05`  
**Range:** `0.0+`

Scaling factor applied to movement speed.

**Formula:** `EffectiveRadius = AcceptanceRadius + (Speed2D * AcceptanceSpeedScale)`

**Tuning Examples:**

```cpp
// Conservative scaling (subtle effect)
AcceptanceSpeedScale = 0.02f;  // 600 speed → +12 radius

// Default scaling (balanced)
AcceptanceSpeedScale = 0.05f;  // 600 speed → +30 radius

// Aggressive scaling (strong effect)
AcceptanceSpeedScale = 0.1f;   // 600 speed → +60 radius
```

#### AcceptanceRadiusMin / AcceptanceRadiusMax

**Properties:** `AcceptanceRadiusMin`, `AcceptanceRadiusMax`  
**Type:** `float`  
**Defaults:** `30.0` / `120.0` units

Bounds for speed-scaled acceptance radius.

**Purpose:**
- **Min:** Prevents radius from becoming too small at low speeds
- **Max:** Prevents radius from becoming too large at high speeds

**Tuning Guidelines:**

```cpp
// Tight control setup
AcceptanceRadiusMin = 20.0f;
AcceptanceRadiusMax = 80.0f;

// Loose control setup
AcceptanceRadiusMin = 40.0f;
AcceptanceRadiusMax = 150.0f;
```

## 🗺️ Pathfinding Configuration

### Lookahead System

Enables corner-cutting behavior for smoother path following.

#### bUseLookahead

**Property:** `bUseLookahead`  
**Type:** `bool`  
**Default:** `true`

Enables blending toward the next path point to smooth turns.

**When Enabled:**
- Character cuts corners smoothly
- More natural movement flow
- Slightly less precise path following

**When Disabled:**
- Character follows path points exactly
- Sharp turns at each waypoint
- More robotic but precise movement

#### LookaheadBlendAlpha

**Property:** `LookaheadBlendAlpha`  
**Type:** `float`  
**Default:** `0.3`  
**Range:** `0.0` - `1.0`

Controls strength of lookahead blending.

**Values:**
- **0.0** - No lookahead (aim only at current target)
- **0.5** - Moderate corner cutting
- **1.0** - Aim directly at next target (may skip waypoints)

**Tuning Guidelines:**

```cpp
// Precise path following
LookaheadBlendAlpha = 0.1f;

// Balanced (default)
LookaheadBlendAlpha = 0.3f;

// Smooth, flowing movement
LookaheadBlendAlpha = 0.5f;

// Aggressive corner cutting (be careful!)
LookaheadBlendAlpha = 0.7f;
```

### Navigation Projection

#### NavProjectExtent

**Property:** `NavProjectExtent`  
**Type:** `FVector`  
**Default:** `(200, 200, 200)`

Half-extents of the search box for navmesh projection.

**Purpose:**
- Converts arbitrary surface clicks to navigable locations
- Larger values find navmesh across bigger gaps
- Smaller values prevent unwanted floor snapping

**Tuning by Level Design:**

```cpp
// Single-floor levels
NavProjectExtent = FVector(150.0f, 150.0f, 50.0f);

// Multi-floor levels
NavProjectExtent = FVector(200.0f, 200.0f, 300.0f);

// Open outdoor levels
NavProjectExtent = FVector(500.0f, 500.0f, 200.0f);

// Tight indoor levels
NavProjectExtent = FVector(100.0f, 100.0f, 100.0f);
```

## 🔍 Collision Configuration

### Cursor Trace Channel

**Property:** `CursorTraceChannel`  
**Type:** `TEnumAsByte<ECollisionChannel>`  
**Default:** `NAVIGATION` (ECC_GameTraceChannel2)

Collision channel used for internal cursor traces.

**Setup Requirements:**
1. Configure channel in Project Settings → Engine → Collision
2. Set ground objects to **Block** this channel
3. Set characters to **Ignore** this channel
4. Set UI/effects to **Ignore** this channel

**Custom Channel Example:**
```cpp
// In your project's collision header
#define MOVEMENT_TRACE ECC_GameTraceChannel3

// In component configuration
CursorTraceChannel = MOVEMENT_TRACE;
```

## 🐛 Debug Configuration

Development and troubleshooting options.

### Debug Visualization

#### bDebugProjectToNav

**Property:** `bDebugProjectToNav`  
**Type:** `bool`  
**Default:** `false`

Enables debug visualization for navmesh projection.

**Visualization:**
- **Cyan Box** - Projection search volume
- **Green Sphere** - Projected result point
- **Green Line** - Connection from input to result

#### Debug Colors and Timing

```cpp
// Debug appearance properties
DebugProjectBoxColor = FColor::Cyan;      // Search box color
DebugProjectedPointColor = FColor::Green; // Result point color
DebugDrawLifetime = 1.5f;                 // Draw duration (seconds)
DebugLineThickness = 1.5f;                // Line thickness
```

### Runtime Debug Commands

**Console Commands:**
```
showdebug ai             // Navigation system debug
showdebug navigation     // NavMesh visualization
stat game               // Performance stats
```

**C++ Debug Setup:**
```cpp
#if UE_BUILD_DEVELOPMENT
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (ClickToMoveComponent)
    {
        // Enable debug visualization
        ClickToMoveComponent->bDebugProjectToNav = true;
        ClickToMoveComponent->DebugDrawLifetime = 3.0f;
    }
}
#endif
```

## 🎮 Game-Specific Configurations

### Action RPG Setup

Fast-paced combat with precise movement:

```cpp
// Fast autorun, tight control
ShortPressThreshold = 0.25f;
AcceptanceRadius = 35.0f;
bScaleAcceptanceBySpeed = true;
AcceptanceSpeedScale = 0.03f;
AcceptanceRadiusMin = 25.0f;
AcceptanceRadiusMax = 60.0f;
bUseLookahead = true;
LookaheadBlendAlpha = 0.2f;
```

### Strategy Game Setup

Deliberate movement with smooth pathing:

```cpp
// Moderate autorun, smooth movement
ShortPressThreshold = 0.5f;
AcceptanceRadius = 60.0f;
bScaleAcceptanceBySpeed = true;
AcceptanceSpeedScale = 0.05f;
AcceptanceRadiusMin = 40.0f;
AcceptanceRadiusMax = 100.0f;
bUseLookahead = true;
LookaheadBlendAlpha = 0.4f;
```

### MMO/World Exploration Setup

Long-distance travel with forgiving controls:

```cpp
// Long autorun, flowing movement
ShortPressThreshold = 0.8f;
AcceptanceRadius = 80.0f;
bScaleAcceptanceBySpeed = true;
AcceptanceSpeedScale = 0.08f;
AcceptanceRadiusMin = 50.0f;
AcceptanceRadiusMax = 150.0f;
bUseLookahead = true;
LookaheadBlendAlpha = 0.5f;
```

### Mobile/Touch Setup

Optimized for touch input precision:

```cpp
// Forgiving controls for touch
ShortPressThreshold = 0.6f;
AcceptanceRadius = 70.0f;
bScaleAcceptanceBySpeed = true;
AcceptanceSpeedScale = 0.06f;
AcceptanceRadiusMin = 50.0f;
AcceptanceRadiusMax = 120.0f;
bUseLookahead = true;
LookaheadBlendAlpha = 0.4f;

// Larger projection for touch imprecision
NavProjectExtent = FVector(300.0f, 300.0f, 200.0f);
```

## ⚡ Performance Considerations

### Optimal Settings

Balance responsiveness with performance:

```cpp
// Recommended for most projects
AcceptanceRadius = 50.0f;           // Good balance
bScaleAcceptanceBySpeed = true;     // Prevent overshoot
NavProjectExtent = FVector(200.0f); // Reasonable search volume
bUseLookahead = true;               // Smooth movement
LookaheadBlendAlpha = 0.3f;         // Moderate corner cutting
```

### Performance Impact Factors

**Low Impact:**
- AcceptanceRadius adjustments
- Lookahead settings
- Speed scaling parameters

**Medium Impact:**
- NavProjectExtent size
- ShortPressThreshold changes

**High Impact:**
- Debug visualization (development only)
- Very frequent projection calls

## 🔧 Runtime Configuration

### Blueprint Runtime Changes

```blueprint
Event BeginPlay
├── Get ClickToMoveComponent
├── Set Short Press Threshold (0.3)
├── Set Acceptance Radius (40.0)
└── Set Use Lookahead (true)
```

### C++ Runtime Changes

```cpp
void AYourPlayerController::ConfigureMovementForGameMode(EGameMode Mode)
{
    if (!ClickToMoveComponent) return;
    
    switch (Mode)
    {
    case EGameMode::Combat:
        ClickToMoveComponent->ShortPressThreshold = 0.2f;
        ClickToMoveComponent->AcceptanceRadius = 30.0f;
        ClickToMoveComponent->LookaheadBlendAlpha = 0.1f;
        break;
        
    case EGameMode::Exploration:
        ClickToMoveComponent->ShortPressThreshold = 0.6f;
        ClickToMoveComponent->AcceptanceRadius = 70.0f;
        ClickToMoveComponent->LookaheadBlendAlpha = 0.4f;
        break;
    }
}
```

### User Preference System

```cpp
void AYourPlayerController::ApplyUserMovementPreferences(const FMovementPreferences& Prefs)
{
    if (!ClickToMoveComponent) return;
    
    // Allow players to customize feel
    ClickToMoveComponent->ShortPressThreshold = Prefs.AutorunSensitivity;
    ClickToMoveComponent->AcceptanceRadius = Prefs.MovementPrecision;
    ClickToMoveComponent->bUseLookahead = Prefs.bSmoothMovement;
}
```

## 🎯 Troubleshooting Configuration Issues

### Issue: Character stops too far from targets

**Solutions:**
- Reduce `AcceptanceRadius`
- Disable `bScaleAcceptanceBySpeed`
- Lower `AcceptanceRadiusMin`

### Issue: Character oscillates around targets

**Solutions:**
- Enable `bScaleAcceptanceBySpeed`
- Increase `AcceptanceRadius`
- Adjust `AcceptanceSpeedScale`

### Issue: Path following looks robotic

**Solutions:**
- Enable `bUseLookahead`
- Increase `LookaheadBlendAlpha`
- Check navmesh quality

### Issue: Autorun triggers too easily/rarely

**Solutions:**
- Adjust `ShortPressThreshold`
- Check input binding timing
- Test on target devices

This configuration guide provides comprehensive coverage of all ClickToMove plugin settings. For setup instructions, see the [Setup Guide](Setup-Guide.md), and for complete API details, see the [API Reference](API-Reference.md).