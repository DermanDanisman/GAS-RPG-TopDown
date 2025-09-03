# Plugin API (Proposed)

Last updated: 2024-12-19

This is the intended surface of the plugin. Namespaces/types can be adapted to your project prefix.

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