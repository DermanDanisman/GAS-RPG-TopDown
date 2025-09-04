# ClickToMove Plugin - Integration Examples

This guide provides practical examples for integrating the ClickToMove plugin with common Unreal Engine systems and frameworks.

## 📚 Table of Contents

- [Enhanced Input System](#enhanced-input-system)
- [Gameplay Ability System (GAS)](#gameplay-ability-system-gas)
- [Custom Interaction Systems](#custom-interaction-systems)
- [Multiplayer Integration](#multiplayer-integration)
- [UI System Integration](#ui-system-integration)
- [Animation System Integration](#animation-system-integration)
- [Custom Extensions](#custom-extensions)

## 🎮 Enhanced Input System

### Basic Integration

Complete setup with Enhanced Input System:

#### Input Assets Setup

1. **Create Input Action Asset**
```
Content/Input/IA_ClickMove
- Value Type: Digital (bool)
```

2. **Input Mapping Context**
```
Content/Input/IMC_PlayerControls
- Add IA_ClickMove → Left Mouse Button
```

#### PlayerController Implementation

```cpp
// YourPlayerController.h
UCLASS()
class YOURGAME_API AYourPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AYourPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UClickToMoveComponent> ClickToMoveComponent;

    // Input Assets
    UPROPERTY(EditAnywhere, Category="Enhanced Input")
    TObjectPtr<UInputMappingContext> InputMappingContext;

    UPROPERTY(EditAnywhere, Category="Enhanced Input")
    TObjectPtr<UInputAction> ClickAction;

    // Input Handlers
    void OnClickStarted(const FInputActionValue& Value);
    void OnClickTriggered(const FInputActionValue& Value);
    void OnClickCompleted(const FInputActionValue& Value);
};
```

```cpp
// YourPlayerController.cpp
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AYourPlayerController::AYourPlayerController()
{
    ClickToMoveComponent = CreateDefaultSubobject<UClickToMoveComponent>(TEXT("ClickToMoveComponent"));
}

void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Add input mapping context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (InputMappingContext)
        {
            Subsystem->AddMappingContext(InputMappingContext, 0);
        }
    }
}

void AYourPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (ClickAction)
        {
            EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &AYourPlayerController::OnClickStarted);
            EIC->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AYourPlayerController::OnClickTriggered);
            EIC->BindAction(ClickAction, ETriggerEvent::Completed, this, &AYourPlayerController::OnClickCompleted);
        }
    }
}

void AYourPlayerController::OnClickStarted(const FInputActionValue& Value)
{
    if (ClickToMoveComponent)
    {
        ClickToMoveComponent->OnClickPressed();
    }
}

void AYourPlayerController::OnClickTriggered(const FInputActionValue& Value)
{
    if (ClickToMoveComponent)
    {
        ClickToMoveComponent->OnClickHeld(true, FHitResult());
    }
}

void AYourPlayerController::OnClickCompleted(const FInputActionValue& Value)
{
    if (ClickToMoveComponent)
    {
        ClickToMoveComponent->OnClickReleased();
    }
}
```

### Advanced Input Context Switching

Switch input behavior based on game state:

```cpp
class YOURGAME_API AYourPlayerController : public APlayerController
{
    // Multiple input contexts for different modes
    UPROPERTY(EditAnywhere, Category="Enhanced Input")
    TObjectPtr<UInputMappingContext> NormalInputContext;

    UPROPERTY(EditAnywhere, Category="Enhanced Input")
    TObjectPtr<UInputMappingContext> CombatInputContext;

    UPROPERTY(EditAnywhere, Category="Enhanced Input")
    TObjectPtr<UInputMappingContext> DialogueInputContext;

public:
    void SetInputMode(EGameInputMode Mode);
};

void AYourPlayerController::SetInputMode(EGameInputMode Mode)
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // Clear all contexts
        Subsystem->ClearAllMappings();
        
        // Add appropriate context
        switch (Mode)
        {
        case EGameInputMode::Normal:
            Subsystem->AddMappingContext(NormalInputContext, 0);
            ClickToMoveComponent->SetIsTargeting(false);
            break;
            
        case EGameInputMode::Combat:
            Subsystem->AddMappingContext(CombatInputContext, 0);
            ClickToMoveComponent->SetIsTargeting(false);
            break;
            
        case EGameInputMode::Dialogue:
            Subsystem->AddMappingContext(DialogueInputContext, 0);
            ClickToMoveComponent->SetIsTargeting(true); // Disable movement
            break;
        }
    }
}
```

## 🎯 Gameplay Ability System (GAS)

### Basic GAS Integration

Coordinate click-to-move with ability targeting:

```cpp
// Enhanced PlayerController with GAS support
class YOURGAME_API AYourPlayerController : public APlayerController
{
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UClickToMoveComponent> ClickToMoveComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UHighlightInteraction> HighlightInteraction;

    // GAS reference
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    // Input handling with GAS priority
    void AbilityInputTagPressed(FGameplayTag InputTag);
    void AbilityInputTagHeld(FGameplayTag InputTag);
    void AbilityInputTagReleased(FGameplayTag InputTag);

    UAbilitySystemComponent* GetASC();
};

void AYourPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
    // Check if this is the left mouse button
    if (InputTag.MatchesTagExact(FYourGameplayTags::Get().InputTag_LMB))
    {
        // Priority: Ability targeting > Movement
        if (HighlightInteraction->GetHighlightedActor())
        {
            // Forward to ability system for targeting
            if (UAbilitySystemComponent* ASC = GetASC())
            {
                ASC->AbilityInputTagHeld(InputTag);
                ClickToMoveComponent->SetIsTargeting(true);
            }
        }
        else
        {
            // Use movement system
            ClickToMoveComponent->SetIsTargeting(false);
            ClickToMoveComponent->OnClickHeld(true, FHitResult());
        }
    }
    else
    {
        // Forward non-LMB inputs to ASC
        if (UAbilitySystemComponent* ASC = GetASC())
        {
            ASC->AbilityInputTagHeld(InputTag);
        }
    }
}

void AYourPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (InputTag.MatchesTagExact(FYourGameplayTags::Get().InputTag_LMB))
    {
        if (HighlightInteraction->GetHighlightedActor())
        {
            if (UAbilitySystemComponent* ASC = GetASC())
            {
                ASC->AbilityInputTagReleased(InputTag);
            }
        }
        else
        {
            ClickToMoveComponent->OnClickReleased();
        }
    }
    else
    {
        if (UAbilitySystemComponent* ASC = GetASC())
        {
            ASC->AbilityInputTagReleased(InputTag);
        }
    }
}
```

### Ability-Driven Movement

Create a movement ability that uses the ClickToMove component:

```cpp
// Custom Gameplay Ability for movement
UCLASS()
class YOURGAME_API UGA_Move : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Move();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    // Configure movement behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float MovementSpeedModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    bool bPreventMovementInterruption = false;
};

UGA_Move::UGA_Move()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Move::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get()))
    {
        if (UClickToMoveComponent* MoveComponent = PC->FindComponentByClass<UClickToMoveComponent>())
        {
            // Configure movement for this ability
            if (bPreventMovementInterruption)
            {
                MoveComponent->SetIsTargeting(true);
            }

            // Apply movement speed modifier
            if (APawn* Pawn = PC->GetPawn())
            {
                if (UCharacterMovementComponent* MovementComp = Pawn->FindComponentByClass<UCharacterMovementComponent>())
                {
                    MovementComp->MaxWalkSpeed *= MovementSpeedModifier;
                }
            }
        }
    }
}
```

### State-Based Movement Control

Control movement based on gameplay tags:

```cpp
void AYourPlayerController::OnGameplayTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    if (!ClickToMoveComponent) return;

    // Disable movement during certain states
    if (Tag.MatchesTag(FYourGameplayTags::Get().State_Stunned) ||
        Tag.MatchesTag(FYourGameplayTags::Get().State_Casting) ||
        Tag.MatchesTag(FYourGameplayTags::Get().State_Dead))
    {
        if (NewCount > 0)
        {
            ClickToMoveComponent->StopMovement();
            ClickToMoveComponent->SetIsTargeting(true);
        }
        else
        {
            ClickToMoveComponent->SetIsTargeting(false);
        }
    }
    
    // Movement speed modifications
    if (Tag.MatchesTag(FYourGameplayTags::Get().State_Slowed))
    {
        // Speed handled by gameplay effects, but could adjust acceptance radius
        if (NewCount > 0)
        {
            ClickToMoveComponent->AcceptanceRadius *= 1.5f; // Larger radius when slowed
        }
        else
        {
            ClickToMoveComponent->AcceptanceRadius /= 1.5f; // Reset radius
        }
    }
}
```

## 🎭 Custom Interaction Systems

### Highlight-Based Interaction

Integrate with object highlighting systems:

```cpp
class YOURGAME_API UHighlightInteraction : public UActorComponent
{
    GENERATED_BODY()

public:
    // Get currently highlighted actor
    AActor* GetHighlightedActor() const { return HighlightedActor; }
    
    // Get hit result from highlight trace
    const FHitResult& GetHighlightHitResult() const { return HighlightHitResult; }

protected:
    UPROPERTY()
    TObjectPtr<AActor> HighlightedActor;

    FHitResult HighlightHitResult;

    void UpdateHighlight();
};

// In PlayerController
void AYourPlayerController::OnClickHeld()
{
    if (HighlightInteraction->GetHighlightedActor())
    {
        // Use hit result from highlight system
        const FHitResult& HighlightHit = HighlightInteraction->GetHighlightHitResult();
        ClickToMoveComponent->OnClickHeld(false, HighlightHit);
    }
    else
    {
        // Use internal navigation trace
        ClickToMoveComponent->OnClickHeld(true, FHitResult());
    }
}
```

### Context-Sensitive Actions

Different actions based on what's under the cursor:

```cpp
void AYourPlayerController::OnClickReleased()
{
    if (AActor* TargetActor = HighlightInteraction->GetHighlightedActor())
    {
        // Interact with different actor types
        if (Cast<AInteractableItem>(TargetActor))
        {
            InteractWithItem(TargetActor);
        }
        else if (Cast<AEnemyCharacter>(TargetActor))
        {
            AttackTarget(TargetActor);
        }
        else if (Cast<ANPCCharacter>(TargetActor))
        {
            StartDialogue(TargetActor);
        }
        else
        {
            // Default to movement
            ClickToMoveComponent->OnClickReleased();
        }
    }
    else
    {
        // No target, use movement
        ClickToMoveComponent->OnClickReleased();
    }
}
```

### Multi-Channel Interaction

Use different trace channels for different systems:

```cpp
class YOURGAME_API AYourPlayerController : public APlayerController
{
    // Different trace channels
    static constexpr ECollisionChannel InteractionChannel = ECC_GameTraceChannel1;
    static constexpr ECollisionChannel MovementChannel = ECC_GameTraceChannel2;
    static constexpr ECollisionChannel UIChannel = ECC_GameTraceChannel3;

    void ProcessClickInput();
};

void AYourPlayerController::ProcessClickInput()
{
    FHitResult InteractionHit;
    FHitResult MovementHit;

    // Trace for interactions first
    bool bInteractionHit = GetHitResultUnderCursor(InteractionChannel, false, InteractionHit);
    
    if (bInteractionHit && InteractionHit.GetActor())
    {
        // Handle interaction
        HandleInteraction(InteractionHit);
    }
    else
    {
        // No interaction, check for movement
        bool bMovementHit = GetHitResultUnderCursor(MovementChannel, false, MovementHit);
        
        if (bMovementHit)
        {
            ClickToMoveComponent->OnClickHeld(false, MovementHit);
        }
        else
        {
            // Fallback to internal trace
            ClickToMoveComponent->OnClickHeld(true, FHitResult());
        }
    }
}
```

## 🌐 Multiplayer Integration

### Client-Side Prediction

The plugin handles client-side movement automatically, but you can add visual feedback:

```cpp
class YOURGAME_API AYourPlayerController : public APlayerController
{
protected:
    // Visual feedback for other players
    UFUNCTION(Server, Reliable)
    void ServerNotifyMovementDestination(const FVector& Destination);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastShowMovementIndicator(const FVector& Destination, APlayerController* RequestingPlayer);

public:
    void OnMovementDestinationSet(const FVector& Destination);
};

void AYourPlayerController::OnMovementDestinationSet(const FVector& Destination)
{
    // Show visual indicator for this player locally
    ShowLocalMovementIndicator(Destination);
    
    // Notify server for other players
    if (IsLocalController())
    {
        ServerNotifyMovementDestination(Destination);
    }
}

void AYourPlayerController::ServerNotifyMovementDestination_Implementation(const FVector& Destination)
{
    // Broadcast to all clients except the sender
    MulticastShowMovementIndicator(Destination, this);
}

void AYourPlayerController::MulticastShowMovementIndicator_Implementation(const FVector& Destination, APlayerController* RequestingPlayer)
{
    // Don't show indicator to the player who initiated movement
    if (RequestingPlayer != this)
    {
        ShowRemotePlayerMovementIndicator(Destination, RequestingPlayer);
    }
}
```

### Authoritative Movement Validation

Server-side validation for competitive games:

```cpp
class YOURGAME_API AYourGameCharacter : public ACharacter
{
protected:
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerValidateMovementDestination(const FVector& Destination);

    // Movement validation rules
    UPROPERTY(EditAnywhere, Category="Movement Validation")
    float MaxMovementRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category="Movement Validation")
    float MinMovementCooldown = 0.1f;

    float LastMovementTime = 0.0f;
};

bool AYourGameCharacter::ServerValidateMovementDestination_Validate(const FVector& Destination)
{
    // Check movement range
    float Distance = FVector::Dist(GetActorLocation(), Destination);
    if (Distance > MaxMovementRange)
    {
        return false;
    }

    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMovementTime < MinMovementCooldown)
    {
        return false;
    }

    // Validate destination is on navmesh
    if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation NavLocation;
        if (!NavSys->ProjectPointToNavigation(Destination, NavLocation))
        {
            return false;
        }
    }

    return true;
}

void AYourGameCharacter::ServerValidateMovementDestination_Implementation(const FVector& Destination)
{
    LastMovementTime = GetWorld()->GetTimeSeconds();
    
    // Additional server-side movement logic
    OnServerMovementValidated(Destination);
}
```

## 🖥️ UI System Integration

### Movement Indicator Widget

Show destination and path to other players:

```cpp
// Movement indicator widget
UCLASS()
class YOURGAME_API UMovementIndicatorWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    UPROPERTY(meta=(BindWidget))
    UImage* DestinationIcon;

    UPROPERTY(meta=(BindWidget))
    UImage* PathLine;

    UPROPERTY(EditAnywhere, Category="Animation")
    TObjectPtr<UWidgetAnimation> FadeInAnimation;

public:
    UFUNCTION(BlueprintCallable)
    void ShowMovementDestination(const FVector& WorldLocation, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable)
    void UpdatePathVisualization(const TArray<FVector>& PathPoints);
};

void UMovementIndicatorWidget::ShowMovementDestination(const FVector& WorldLocation, float Duration)
{
    // Convert world location to screen position
    if (APlayerController* PC = GetOwningPlayer())
    {
        FVector2D ScreenPosition;
        if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition))
        {
            // Position indicator at screen location
            SetPositionInViewport(ScreenPosition);
            
            // Play animation
            if (FadeInAnimation)
            {
                PlayAnimation(FadeInAnimation);
            }
            
            // Auto-hide after duration
            GetWorld()->GetTimerManager().SetTimer(
                FadeOutTimer, 
                this, 
                &UMovementIndicatorWidget::FadeOut, 
                Duration, 
                false
            );
        }
    }
}
```

### HUD Integration

Display movement state in game HUD:

```cpp
class YOURGAME_API AYourGameHUD : public AHUD
{
protected:
    virtual void DrawHUD() override;

    // Movement state display
    void DrawMovementInfo();
    
    UPROPERTY(EditAnywhere, Category="HUD")
    TSubclassOf<UMovementIndicatorWidget> MovementIndicatorClass;

    UPROPERTY()
    TObjectPtr<UMovementIndicatorWidget> MovementIndicator;

public:
    void ShowMovementDestination(const FVector& Destination);
    void HideMovementDestination();
};

void AYourGameHUD::DrawMovementInfo()
{
    if (APlayerController* PC = GetOwningPlayerController())
    {
        if (UClickToMoveComponent* MoveComp = PC->FindComponentByClass<UClickToMoveComponent>())
        {
            if (MoveComp->IsAutoRunning())
            {
                // Draw path visualization
                const TArray<FVector>& PathPoints = MoveComp->GetPathPoints();
                
                for (int32 i = 0; i < PathPoints.Num() - 1; i++)
                {
                    FVector2D ScreenStart, ScreenEnd;
                    if (Project(PathPoints[i], ScreenStart) && Project(PathPoints[i + 1], ScreenEnd))
                    {
                        DrawLine(ScreenStart, ScreenEnd, FLinearColor::Green, 2.0f);
                    }
                }
                
                // Draw destination marker
                FVector2D DestScreen;
                if (Project(MoveComp->GetCachedDestination(), DestScreen))
                {
                    DrawRect(FLinearColor::Yellow, DestScreen.X - 5, DestScreen.Y - 5, 10, 10);
                }
            }
        }
    }
}
```

## 🎬 Animation System Integration

### Movement Animation Coordination

Coordinate animations with movement state:

```cpp
class YOURGAME_API UYourAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

protected:
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Movement state variables for animation
    UPROPERTY(BlueprintReadOnly, Category="Movement")
    bool bIsAutoRunning = false;

    UPROPERTY(BlueprintReadOnly, Category="Movement")
    bool bIsTargeting = false;

    UPROPERTY(BlueprintReadOnly, Category="Movement")
    float MovementSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Movement")
    FVector MovementDirection = FVector::ZeroVector;

    // References
    UPROPERTY()
    TObjectPtr<UClickToMoveComponent> ClickToMoveComponent;

    UPROPERTY()
    TObjectPtr<APawn> OwnerPawn;
};

void UYourAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
    }

    if (!ClickToMoveComponent)
    {
        if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        {
            ClickToMoveComponent = PC->FindComponentByClass<UClickToMoveComponent>();
        }
        if (!ClickToMoveComponent) return;
    }

    // Update movement state for animations
    bIsAutoRunning = ClickToMoveComponent->IsAutoRunning();
    bIsTargeting = ClickToMoveComponent->IsTargeting();
    
    // Calculate movement variables
    FVector Velocity = OwnerPawn->GetVelocity();
    MovementSpeed = Velocity.Size2D();
    MovementDirection = Velocity.GetSafeNormal2D();
}
```

### Arrival Animation Triggers

Trigger animations when reaching destinations:

```cpp
class YOURGAME_API UClickToMoveComponent : public UActorComponent
{
public:
    // Delegate for arrival events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArrivalDelegate, const FVector&, Destination);

    UPROPERTY(BlueprintAssignable, Category="Movement|Events")
    FOnArrivalDelegate OnArrival;

protected:
    void AutoRun() override;
};

void UClickToMoveComponent::AutoRun()
{
    // ... existing autorun logic ...

    // When reaching final destination
    if (PathIndex >= PathPoints.Num())
    {
        FVector FinalDestination = CachedDestination;
        StopMovement();
        
        // Broadcast arrival event
        OnArrival.Broadcast(FinalDestination);
        return;
    }
}

// In character or animation blueprint
void AYourCharacter::OnMovementArrival(const FVector& Destination)
{
    // Play arrival animation
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        AnimInstance->Montage_Play(ArrivalMontage);
    }
    
    // Trigger arrival effects
    if (ArrivalParticleSystem)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ArrivalParticleSystem, Destination);
    }
}
```

## 🔧 Custom Extensions

### Custom Movement Component

Extend the base component for specialized behavior:

```cpp
UCLASS()
class YOURGAME_API UAdvancedClickToMoveComponent : public UClickToMoveComponent
{
    GENERATED_BODY()

public:
    UAdvancedClickToMoveComponent();

protected:
    virtual void AutoRun() override;
    virtual void FindPathToLocation() override;

    // Advanced features
    UPROPERTY(EditAnywhere, Category="Advanced Movement")
    bool bAvoidOtherCharacters = true;

    UPROPERTY(EditAnywhere, Category="Advanced Movement")
    float AvoidanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, Category="Advanced Movement")
    TArray<TEnumAsByte<ECollisionChannel>> AvoidanceChannels;

    // Custom pathfinding
    void PerformAvoidanceCheck(FVector& AimPoint);
    bool FindAlternatePath(const FVector& BlockedDestination, FVector& AlternateDestination);
};

void UAdvancedClickToMoveComponent::AutoRun()
{
    // Call parent implementation
    Super::AutoRun();

    // Add avoidance behavior
    if (bAvoidOtherCharacters && bIsAutoRunning)
    {
        FVector CurrentAimPoint = GetCurrentAimPoint();
        PerformAvoidanceCheck(CurrentAimPoint);
    }
}

void UAdvancedClickToMoveComponent::PerformAvoidanceCheck(FVector& AimPoint)
{
    APawn* Pawn = GetControlledPawn();
    if (!Pawn) return;

    // Check for obstacles in movement direction
    FVector Start = Pawn->GetActorLocation();
    FVector End = AimPoint;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Pawn);
    
    FHitResult HitResult;
    if (GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        AvoidanceChannels[0],
        FCollisionShape::MakeSphere(AvoidanceRadius),
        QueryParams))
    {
        // Adjust aim point to avoid obstacle
        FVector AvoidanceDirection = (HitResult.ImpactNormal + FVector::UpVector).GetSafeNormal();
        AimPoint += AvoidanceDirection * AvoidanceRadius;
    }
}
```

### Movement State Machine

Implement complex movement behavior with state machines:

```cpp
UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle,
    Moving,
    Attacking,
    Stunned,
    Following
};

UCLASS()
class YOURGAME_API UMovementStateMachine : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SetMovementState(EMovementState NewState);

    UFUNCTION(BlueprintPure)
    EMovementState GetCurrentState() const { return CurrentState; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    EMovementState CurrentState = EMovementState::Idle;

    UPROPERTY()
    TObjectPtr<UClickToMoveComponent> ClickToMoveComponent;

    virtual void BeginPlay() override;

    // State transition handlers
    void OnEnterState(EMovementState State);
    void OnExitState(EMovementState State);
};

void UMovementStateMachine::SetMovementState(EMovementState NewState)
{
    if (NewState == CurrentState) return;

    OnExitState(CurrentState);
    CurrentState = NewState;
    OnEnterState(NewState);
}

void UMovementStateMachine::OnEnterState(EMovementState State)
{
    if (!ClickToMoveComponent) return;

    switch (State)
    {
    case EMovementState::Idle:
        ClickToMoveComponent->StopMovement();
        ClickToMoveComponent->SetIsTargeting(false);
        break;

    case EMovementState::Moving:
        ClickToMoveComponent->SetIsTargeting(false);
        break;

    case EMovementState::Attacking:
    case EMovementState::Stunned:
        ClickToMoveComponent->StopMovement();
        ClickToMoveComponent->SetIsTargeting(true);
        break;

    case EMovementState::Following:
        // Custom following logic
        break;
    }
}
```

These integration examples demonstrate how to effectively combine the ClickToMove plugin with various Unreal Engine systems. Each example can be adapted to fit your specific project needs and requirements.

For more information, see the [Setup Guide](Setup-Guide.md), [API Reference](API-Reference.md), and [Configuration Guide](Configuration-Guide.md).