#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Input/TDInputConfig.h"
#include "AuraInputComponent.generated.h"

/**
 * Custom input component that binds ability input actions from a data-driven config.
 */
UCLASS(ClassGroup=(Input))
class RPG_TOPDOWN_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/**
	 * Bind Pressed/Released/Held callbacks for every entry in the provided input config.
	 * Each callback receives the entry's InputTag as an extra argument.
	 *
	 * Example usage (in a PlayerController):
	 *   UAuraInputComponent* AIC = CastChecked<UAuraInputComponent>(InputComponent);
	 *   AIC->BindAbilityActions(InputConfig, this,
	 *       &AMyPC::OnAbilityInputPressed,
	 *       &AMyPC::OnAbilityInputReleased,
	 *       &AMyPC::OnAbilityInputHeld);
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UTDInputConfig* InputConfig,
						   UserClass* Object,
						   PressedFuncType PressedFunc,
						   ReleasedFuncType ReleasedFunc,
						   HeldFuncType HeldFunc)
	{
		if (!InputConfig)
		{
			UE_LOG(LogTemp, Error, TEXT("BindAbilityActions: InputConfig is null on %s"), *GetNameSafe(this));
			return;
		}

		if (InputConfig->AbilityInputActions.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("BindAbilityActions: AbilityInputActions is empty on %s"), *GetNameSafe(InputConfig));
			return;
		}

		for (const FTDInputAction& Action : InputConfig->AbilityInputActions)
		{
			if (!Action.InputAction || !Action.InputTag.IsValid())
			{
				continue;
			}

			// Pressed once at start
			if (PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}

			// Released when input completes
			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}

			// Called every frame while held
			if (HeldFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
};