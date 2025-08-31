// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "GASCoreAbilityInitComponent.generated.h"

class UGameplayAbility;

/**
 * UGASCoreAbilityInitComponent
 *
 * Actor component that handles initialization and granting of gameplay abilities
 * to characters at startup.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASCORE_API UGASCoreAbilityInitComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UGASCoreAbilityInitComponent();

	/** Grants all abilities in StartupAbilities to the owning actor's AbilitySystemComponent. */
	virtual void AddCharacterAbilities();

protected:

	/** Array of ability classes to grant to the owner at startup. */
	UPROPERTY(EditAnywhere, Category="GASCore|Ability Init Component|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
};
