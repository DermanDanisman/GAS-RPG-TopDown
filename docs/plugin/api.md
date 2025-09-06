# Plugin API (Proposed)

Last updated: 2025-01-21

This is the intended surface of the plugin. Namespaces/types can be adapted to your project prefix.

## Core Ability System Classes

### UGASCoreGameplayAbility : UGameplayAbility

Base class for all gameplay abilities in the GASCore plugin.

- `UPROPERTY(EditDefaultsOnly) FGameplayTag StartupInputTag;`
- `UFUNCTION(BlueprintPure) TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> GetSpawnActorClass();`
- `UFUNCTION(BlueprintCallable) virtual void SpawnActorFromGameplayAbility();`
- `UPROPERTY(EditAnywhere) TSubclassOf<AGASCoreSpawnedActorByGameplayAbility> SpawnActorClass;`

### UGASCoreProjectileAbility : UGASCoreGameplayAbility

Specialized ability class for projectile-based abilities.

- `virtual void ActivateAbility(...) override;`
- `virtual void SpawnActorFromGameplayAbility() override;`

### AGASCoreSpawnedActorByGameplayAbility : AActor

Base class for actors spawned by gameplay abilities (primarily projectiles).

- `UFUNCTION(BlueprintPure) UProjectileMovementComponent* GetProjectileMovementComponent();`
- `UFUNCTION(BlueprintCallable) void SetDamageEffectSpec(const FGameplayEffectSpecHandle& InDamageEffect);`
- `UFUNCTION(BlueprintCallable) void ApplyEffectToTarget(AActor* Target);`
- `UFUNCTION(BlueprintCallable) void SetLifetime(float InLifetime);`
- `UFUNCTION(BlueprintNativeEvent) bool IsValidTarget(AActor* PotentialTarget) const;`

## Ability Tasks

### UGASCoreTargetDataFromAimTrace : UAbilityTask

Ability task for target selection through aim tracing.

- `UFUNCTION(BlueprintCallable) static UGASCoreTargetDataFromAimTrace* CreateTargetDataFromAimTrace(UGameplayAbility* OwningAbility);`
- `UPROPERTY(BlueprintAssignable) FAimTraceTargetDataSignature ValidHitResultData;`
- Delegate: `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimTraceTargetDataSignature, const FHitResult&, HitResultData);`

## Legacy Classes (To Be Integrated)

### UTDAbilitySystemComponent : UAbilitySystemComponent

- void AbilityActorInfoSet();
- FEffectAssetTags EffectAssetTags; // multicast delegate with const FGameplayTagContainer&

Binding example:
```cpp
OnGameplayEffectAppliedDelegateToSelf.AddUObject(
  this, &UTDAbilitySystemComponent::OnEffectApplied);
```

### UTDAttributeSet : UAttributeSet

- ATTRIBUTE_ACCESSORS for Health/MaxHealth/Mana/MaxMana
- Constructor initializes defaults
- Optional clamp hooks

### UTDUserWidget : UUserWidget

- UFUNCTION(BlueprintCallable) void SetWidgetController(UObject* InController);
- UFUNCTION(BlueprintImplementableEvent) void WidgetControllerSet();

### UTDWidgetController : UObject

- UPROPERTY(BlueprintReadOnly) APlayerController* PlayerController;
- UPROPERTY(BlueprintReadOnly) APlayerState* PlayerState;
- UPROPERTY(BlueprintReadOnly) UAbilitySystemComponent* AbilitySystemComponent;
- UPROPERTY(BlueprintReadOnly) UAttributeSet* AttributeSet;

- virtual void BroadcastInitialValues();
- virtual void BindCallbacksToDependencies();

### ATDEffectActor : AActor

- UPROPERTY(EditAnywhere) TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
- UPROPERTY(EditAnywhere) TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
- UPROPERTY(EditAnywhere) TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;

- UPROPERTY(EditAnywhere) EEffectApplicationPolicy InstantEffectApplicationPolicy;
- UPROPERTY(EditAnywhere) EEffectApplicationPolicy DurationEffectApplicationPolicy;
- UPROPERTY(EditAnywhere) EEffectApplicationPolicy InfiniteEffectApplicationPolicy;

- UPROPERTY(EditAnywhere) EEffectRemovalPolicy InfiniteEffectRemovalPolicy;
- UPROPERTY(EditAnywhere) bool bDestroyOnEffectRemoval = false;

## UTDAbilitySystemComponent : UAbilitySystemComponent

- void AbilityActorInfoSet();
- FEffectAssetTags EffectAssetTags; // multicast delegate with const FGameplayTagContainer&

Binding example:
```cpp
OnGameplayEffectAppliedDelegateToSelf.AddUObject(
  this, &UTDAbilitySystemComponent::OnEffectApplied);
```

## UTDAttributeSet : UAttributeSet

- ATTRIBUTE_ACCESSORS for Health/MaxHealth/Mana/MaxMana
- Constructor initializes defaults
- Optional clamp hooks

## UTDUserWidget : UUserWidget

- UFUNCTION(BlueprintCallable) void SetWidgetController(UObject* InController);
- UFUNCTION(BlueprintImplementableEvent) void WidgetControllerSet();

## UTDWidgetController : UObject

- UPROPERTY(BlueprintReadOnly) APlayerController* PlayerController;
- UPROPERTY(BlueprintReadOnly) APlayerState* PlayerState;
- UPROPERTY(BlueprintReadOnly) UAbilitySystemComponent* AbilitySystemComponent;
- UPROPERTY(BlueprintReadOnly) UAttributeSet* AttributeSet;

- virtual void BroadcastInitialValues();
- virtual void BindCallbacksToDependencies();

## ATDEffectActor : AActor

- UPROPERTY(EditAnywhere) TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
- UPROPERTY(EditAnywhere) TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
- UPROPERTY(EditAnywhere) TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;

- UPROPERTY(EditAnywhere) EEffectApplicationPolicy InstantEffectApplicationPolicy;
- UPROPERTY(EditAnywhere) EEffectApplicationPolicy DurationEffectApplicationPolicy;
- UPROPERTY(EditAnywhere) EEffectApplicationPolicy InfiniteEffectApplicationPolicy;

- UPROPERTY(EditAnywhere) EEffectRemovalPolicy InfiniteEffectRemovalPolicy;
- UPROPERTY(EditAnywhere) bool bDestroyOnEffectRemoval = false;

- UFUNCTION(BlueprintCallable) void OnOverlap(AActor* TargetActor);
- UFUNCTION(BlueprintCallable) void OnEndOverlap(AActor* TargetActor);