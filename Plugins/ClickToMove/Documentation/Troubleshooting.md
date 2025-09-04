# ClickToMove Plugin - Troubleshooting Guide

This guide helps you diagnose and resolve common issues with the ClickToMove plugin.

## 🚨 Quick Diagnostics

### First Steps Checklist

Before diving into specific issues, verify these basics:

1. **Plugin Enabled**: Check Edit → Plugins → ClickToMove is enabled
2. **Component Added**: ClickToMoveComponent is added to PlayerController
3. **Input Bound**: Mouse input is properly connected to component functions
4. **NavMesh Present**: Level has NavMeshBoundsVolume with visible green navmesh
5. **Collision Setup**: NAVIGATION channel configured and objects respond correctly

### Debug Commands

Enable these console commands for debugging:

```
showdebug ai              // Show AI/Navigation debug
showdebug navigation      // Show NavMesh visualization  
stat game                 // Performance statistics
stat engine               // Engine performance
```

### Debug Visualization

Enable debug mode on the component:

```cpp
// C++ runtime enable
ClickToMoveComponent->bDebugProjectToNav = true;

// Blueprint: Set "Debug Project To Nav" to true in component details
```

## 🚫 Movement Issues

### Issue: Character doesn't move at all

**Symptoms:**
- Clicking has no effect
- No debug visualization appears
- Character remains stationary

**Solutions:**

1. **Verify Input Binding**
```cpp
// Check that component functions are actually being called
void AYourPlayerController::OnClickPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("OnClickPressed called")); // Add this line
    if (ClickToMoveComponent)
    {
        ClickToMoveComponent->OnClickPressed();
    }
}
```

2. **Check Component Reference**
```cpp
// In BeginPlay or constructor
if (!ClickToMoveComponent)
{
    UE_LOG(LogTemp, Error, TEXT("ClickToMoveComponent is null!"));
}
```

3. **Verify Enhanced Input Setup**
```cpp
// Ensure Input Mapping Context is added
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (InputMappingContext)
        {
            Subsystem->AddMappingContext(InputMappingContext, 0);
            UE_LOG(LogTemp, Warning, TEXT("Input context added successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InputMappingContext is null!"));
        }
    }
}
```

4. **Check NavMesh**
- Add NavMeshBoundsVolume to level
- Scale to cover play area
- Verify green navmesh is visible in viewport
- Check NavMesh settings in Project Settings

### Issue: Character moves but doesn't reach destination

**Symptoms:**
- Character starts moving but stops before reaching target
- Autorun ends prematurely
- Character circles around destination

**Solutions:**

1. **Adjust Acceptance Radius**
```cpp
// Try reducing acceptance radius for tighter control
ClickToMoveComponent->AcceptanceRadius = 25.0f;

// Or disable speed scaling if it's too aggressive
ClickToMoveComponent->bScaleAcceptanceBySpeed = false;
```

2. **Check Navmesh Quality**
- Ensure navmesh covers the destination area
- Look for navmesh gaps or holes
- Verify navmesh generation settings in Project Settings

3. **Debug Path Points**
```cpp
// Enable debug to see path visualization
ClickToMoveComponent->bDebugProjectToNav = true;
```

4. **Verify Collision Channel**
```cpp
// Check that NAVIGATION channel is set up correctly
// Ground should BLOCK, character should IGNORE
```

### Issue: Character doesn't follow optimal path

**Symptoms:**
- Character takes unnecessarily long routes
- Doesn't cut corners smoothly
- Gets stuck on simple obstacles

**Solutions:**

1. **Enable Lookahead**
```cpp
ClickToMoveComponent->bUseLookahead = true;
ClickToMoveComponent->LookaheadBlendAlpha = 0.3f; // Adjust 0.1-0.5
```

2. **Check NavMesh Generation**
- Increase navmesh cell size for smoother paths
- Verify Agent Radius matches character capsule
- Check walkable slope angles

3. **Adjust Path Optimization**
```cpp
// Increase acceptance radius slightly for smoother flow
ClickToMoveComponent->AcceptanceRadius = 60.0f;
```

## ⚡ Performance Issues

### Issue: Frame rate drops during movement

**Symptoms:**
- Stuttering during pathfinding
- Hitches when clicking to move
- High CPU usage in profiler

**Solutions:**

1. **Verify Conditional Ticking**
```cpp
// Component should only tick during autorun
void UClickToMoveComponent::AutoRun()
{
    // Ensure this only runs when bIsAutoRunning is true
    if (!bIsAutoRunning)
    {
        UE_LOG(LogTemp, Error, TEXT("AutoRun called when not auto running!"));
        return;
    }
    // ... rest of function
}
```

2. **Optimize NavMesh Settings**
- Reduce navmesh tile size if very detailed
- Increase cell size for less precise but faster paths
- Limit NavMesh generation to playable areas only

3. **Debug Visualization Impact**
```cpp
// Disable debug drawing in shipping/performance testing
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (bDebugProjectToNav)
    {
        // Debug drawing code
    }
#endif
```

4. **Check Trace Frequency**
```cpp
// Ensure traces only happen when needed
void AYourPlayerController::OnClickHeld()
{
    // Don't trace if already targeting
    if (ClickToMoveComponent->IsTargeting())
    {
        return; // Early out
    }
    
    ClickToMoveComponent->OnClickHeld(true, FHitResult());
}
```

### Issue: Memory usage increases over time

**Symptoms:**
- Memory usage grows during play
- Array sizes increase without bounds
- Garbage collection hitches

**Solutions:**

1. **Check Path Array Cleanup**
```cpp
// Verify paths are cleared properly
void UClickToMoveComponent::StopMovement()
{
    PathPoints.Reset(); // Should be called
    PathIndex = INDEX_NONE;
}
```

2. **Monitor Spline Component**
```cpp
// Spline should be unregistered (no scene cost)
USplineComponent* UClickToMoveComponent::EnsureSplineNoAttach()
{
    if (!Spline)
    {
        Spline = NewObject<USplineComponent>(GetOwner(), TEXT("ClickToMoveSpline"));
        // Should NOT call RegisterComponent() or AttachToComponent()
    }
    return Spline;
}
```

## 🌐 Multiplayer Issues

### Issue: Movement desyncs between clients

**Symptoms:**
- Character appears in different positions on different clients
- Rubber-banding movement
- Inconsistent movement behavior

**Solutions:**

1. **Verify Local Controller Check**
```cpp
void UClickToMoveComponent::AutoRun()
{
    // Must check for local controller
    const APlayerController* PC = GetOwnerPC();
    if (!PC || !PC->IsLocalController())
    {
        return; // Don't execute movement on non-local controllers
    }
    // ... rest of function
}
```

2. **Check CharacterMovement Replication**
```cpp
// In Character constructor
GetCharacterMovement()->SetIsReplicated(true);
GetCharacterMovement()->SetDefaultMovementMode(MOVE_Walking);
```

3. **Consistent NavMesh Across Clients**
- Ensure all clients have identical navmesh data
- Check navmesh generation is deterministic
- Verify level streaming doesn't affect navmesh

### Issue: Server authority conflicts

**Symptoms:**
- Movement commands ignored on server
- Client predictions fail
- Authority warnings in logs

**Solutions:**

1. **Client-Side Movement Only**
```cpp
// Movement should only be driven client-side
void UClickToMoveComponent::ApplyMoveToward(const FVector& Destination) const
{
    const APlayerController* PC = GetOwnerPC();
    if (!PC || !PC->IsLocalController())
    {
        return; // Never apply movement on server/remote clients
    }
    
    if (APawn* Pawn = GetControlledPawn())
    {
        const FVector Direction = (Destination - Pawn->GetActorLocation()).GetSafeNormal();
        Pawn->AddMovementInput(Direction, 1.0f);
    }
}
```

2. **Optional Server Validation**
```cpp
// Add server RPC for validation if needed
UFUNCTION(Server, Reliable, WithValidation)
void ServerValidateMovement(const FVector& Destination);
```

## 🔗 Integration Issues

### Issue: Conflicts with Gameplay Ability System

**Symptoms:**
- Movement interferes with ability targeting
- Abilities don't activate when expected
- Input priority conflicts

**Solutions:**

1. **Proper Targeting Mode**
```cpp
void AYourPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
    if (InputTag.MatchesTagExact(FGameplayTags::Get().InputTag_LMB))
    {
        // Check targeting first
        if (HighlightInteraction->GetHighlightedActor())
        {
            // Disable movement during targeting
            ClickToMoveComponent->SetIsTargeting(true);
            // Forward to ability system
            if (GetASC())
            {
                GetASC()->AbilityInputTagHeld(InputTag);
            }
        }
        else
        {
            // Enable movement
            ClickToMoveComponent->SetIsTargeting(false);
            ClickToMoveComponent->OnClickHeld(true, FHitResult());
        }
    }
}
```

2. **Check Input Priority**
```cpp
// Process inputs in correct order
void AYourPlayerController::OnClickHeld()
{
    // 1. Check for UI interactions
    if (IsUIFocused()) { return; }
    
    // 2. Check for ability targeting
    if (IsTargetingAbility()) 
    { 
        ForwardToAbilitySystem();
        return; 
    }
    
    // 3. Handle movement
    ClickToMoveComponent->OnClickHeld(true, FHitResult());
}
```

### Issue: Enhanced Input conflicts

**Symptoms:**
- Input events fire multiple times
- Wrong trigger events activated
- Input mapping context conflicts

**Solutions:**

1. **Check Trigger Event Types**
```cpp
// Use correct trigger events
EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &APC::OnClickPressed);
EIC->BindAction(ClickAction, ETriggerEvent::Triggered, this, &APC::OnClickHeld);
EIC->BindAction(ClickAction, ETriggerEvent::Completed, this, &APC::OnClickReleased);
```

2. **Context Priority**
```cpp
// Set appropriate priority for input contexts
Subsystem->AddMappingContext(UIContext, 1);       // Highest priority
Subsystem->AddMappingContext(GameplayContext, 0); // Lower priority
```

3. **Consume Input When Appropriate**
```cpp
// Prevent input passthrough in UI
void AYourPlayerController::OnClickPressed()
{
    if (IsUIFocused())
    {
        // Don't forward to movement system
        return;
    }
    
    ClickToMoveComponent->OnClickPressed();
}
```

## 🎯 Navigation Issues

### Issue: Clicks on non-walkable surfaces don't work

**Symptoms:**
- Clicking walls/obstacles has no effect
- No movement on certain surfaces
- Inconsistent click response

**Solutions:**

1. **Check NavMesh Projection**
```cpp
// Verify projection is working
bool UClickToMoveComponent::ProjectPointToNavmesh(const FVector& InWorld, FVector& OutProjected) const
{
    // Enable debug to see projection attempts
    if (bDebugProjectToNav)
    {
        UE_LOG(LogTemp, Warning, TEXT("Projecting point: %s"), *InWorld.ToString());
    }
    
    // ... projection logic
    
    if (bProjected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Projection successful: %s"), *OutProjected.ToString());
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Projection failed for: %s"), *InWorld.ToString());
        return false;
    }
}
```

2. **Adjust Projection Extents**
```cpp
// Increase search area for projection
ClickToMoveComponent->NavProjectExtent = FVector(300.0f, 300.0f, 500.0f);
```

3. **Verify Collision Channel Setup**
```cpp
// Check trace channel configuration
// NAVIGATION channel should:
// - Block on ground/floor objects
// - Ignore on characters
// - Block on walls (optional, for projection)
```

### Issue: NavMesh generation problems

**Symptoms:**
- No green navmesh visible
- Gaps in navigation
- Characters fall through world

**Solutions:**

1. **Check NavMesh Settings**
- Project Settings → Engine → Navigation System
- Mode: Automatic or Custom
- Generate Navigation Data: Enabled

2. **NavMeshBoundsVolume Setup**
- Add to level covering all walkable areas
- Check volume encompasses all floors/platforms
- Verify volume is not rotated unusually

3. **Agent Configuration**
```cpp
// In Project Settings → Navigation System
Agent Radius: 34.0      // Match character capsule
Agent Height: 144.0     // Match character height
Step Height: 35.0       // Stair climbing ability
Max Slope: 44.9         // Walkable slope angle
```

## 🐛 Component State Issues

### Issue: Component stuck in wrong state

**Symptoms:**
- bIsAutoRunning stays true when it shouldn't
- Targeting mode doesn't clear
- Component ticking when it shouldn't

**Solutions:**

1. **Force State Reset**
```cpp
// Add debug function to reset component state
void UClickToMoveComponent::ResetState()
{
    StopMovement();
    SetIsTargeting(false);
    FollowTime = 0.0f;
    CachedDestination = FVector::ZeroVector;
    SetComponentTickEnabled(false);
}
```

2. **Debug State Monitoring**
```cpp
// Add to TickComponent for debugging
void UClickToMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    #if UE_BUILD_DEVELOPMENT
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("AutoRunning: %s, Targeting: %s, PathPoints: %d"),
                bIsAutoRunning ? TEXT("True") : TEXT("False"),
                bIsTargeting ? TEXT("True") : TEXT("False"),
                PathPoints.Num()));
    }
    #endif
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
```

3. **State Validation**
```cpp
// Add state consistency checks
void UClickToMoveComponent::ValidateState()
{
    // If not auto running, shouldn't be ticking
    if (!bIsAutoRunning && IsComponentTickEnabled())
    {
        UE_LOG(LogTemp, Warning, TEXT("Component ticking when not auto running!"));
        SetComponentTickEnabled(false);
    }
    
    // If no path points, shouldn't be auto running
    if (PathPoints.IsEmpty() && bIsAutoRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto running with no path points!"));
        SetAutoRunActive(false);
    }
}
```

## 📊 Performance Profiling

### Profiling Movement Performance

Use these tools to identify performance bottlenecks:

1. **Stat Commands**
```
stat game                // Overall game performance
stat engine              // Engine subsystems
stat navigation          // Navigation system performance
stat memory              // Memory usage
```

2. **Unreal Insights**
- Enable Insights in project
- Capture traces during movement
- Look for spikes in ClickToMove component functions

3. **Custom Profiling**
```cpp
void UClickToMoveComponent::AutoRun()
{
    SCOPE_CYCLE_COUNTER(STAT_ClickToMoveAutoRun);
    
    // ... function implementation
}

// Add to .cpp file
DECLARE_CYCLE_STAT(TEXT("ClickToMove AutoRun"), STAT_ClickToMoveAutoRun, STATGROUP_Game);
```

## 🔍 Debug Logging

### Useful Debug Logging

Add these logs for troubleshooting:

```cpp
// In key functions, add logging
void UClickToMoveComponent::OnClickPressed()
{
    UE_LOG(LogClickToMove, Log, TEXT("OnClickPressed - Stopping autorun"));
    SetAutoRunActive(false);
    // ...
}

void UClickToMoveComponent::OnClickHeld(const bool bUseInternalHitResult, const FHitResult& InHitResult)
{
    UE_LOG(LogClickToMove, Verbose, TEXT("OnClickHeld - UseInternal: %s, FollowTime: %.2f"), 
        bUseInternalHitResult ? TEXT("true") : TEXT("false"), FollowTime);
    // ...
}

void UClickToMoveComponent::FindPathToLocation()
{
    UE_LOG(LogClickToMove, Log, TEXT("FindPathToLocation - FollowTime: %.2f, Threshold: %.2f"), 
        FollowTime, ShortPressThreshold);
    // ...
}

// Define log category in header
DECLARE_LOG_CATEGORY_EXTERN(LogClickToMove, Log, All);

// In .cpp file
DEFINE_LOG_CATEGORY(LogClickToMove);
```

## 📞 Getting Help

### Information to Provide

When seeking help, include:

1. **Engine Version**: UE5.x.x
2. **Plugin Version**: Check .uplugin file
3. **Platform**: Windows/Mac/Linux
4. **Project Type**: Blueprint/C++/Mixed
5. **Error Messages**: Full log output
6. **Reproduction Steps**: Minimal example
7. **Expected vs Actual Behavior**: Clear description

### Debug Information Gathering

```cpp
// Function to dump component state for support
void UClickToMoveComponent::DumpDebugInfo()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ClickToMove Debug Info ==="));
    UE_LOG(LogTemp, Warning, TEXT("IsAutoRunning: %s"), bIsAutoRunning ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("IsTargeting: %s"), bIsTargeting ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("PathPoints Count: %d"), PathPoints.Num());
    UE_LOG(LogTemp, Warning, TEXT("PathIndex: %d"), PathIndex);
    UE_LOG(LogTemp, Warning, TEXT("FollowTime: %.2f"), FollowTime);
    UE_LOG(LogTemp, Warning, TEXT("CachedDestination: %s"), *CachedDestination.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Component Ticking: %s"), IsComponentTickEnabled() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}
```

This troubleshooting guide covers the most common issues with the ClickToMove plugin. For additional help, refer to the [Setup Guide](Setup-Guide.md), [API Reference](API-Reference.md), and [Configuration Guide](Configuration-Guide.md).