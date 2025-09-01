// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "TDInputConfig.h"
#include "TDEnhancedInputComponent.generated.h"

/**
 * Custom input component that binds ability input actions from a data-driven config.
 * Provides template-based binding for input delegates using tags.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_TOPDOWN_API UTDEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTDEnhancedInputComponent();

	/**
	 * Binds all ability input actions defined in the input config to the given user class' handler functions.
	 *
	 * @param InputConfig Data asset defining input actions and tags.
	 * @param Object The object (usually a player controller) to bind delegates to.
	 * @param PressedFunc Handler for "Pressed/Started" (may be nullptr or invalid if not used).
	 * @param ReleasedFunc Handler for "Released/Completed" (may be nullptr or invalid if not used).
	 * @param HeldFunc Handler for "Held/Triggered" (may be nullptr or invalid if not used).
	 *
	 * Note: Use function pointer null checks, not .IsValid() (which is not valid for raw pointers).
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityInputActions(const UTDInputConfig* InputConfig, UserClass* Object,
		PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UTDEnhancedInputComponent::BindAbilityInputActions(const UTDInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	for (const FTDInputAction& InputAction : InputConfig->AbilityInputActions)
	{
		// Only bind if both an InputAction asset and a valid InputTag are present.
		if (InputAction.InputAction && InputAction.InputTag.IsValid())
		{
			// For function pointers, check for nullptr (not .IsValid()).
			if (PressedFunc)
			{
				BindAction(InputAction.InputAction, ETriggerEvent::Started, Object, PressedFunc, InputAction.InputTag);	
			}
			if (ReleasedFunc)
			{
				BindAction(InputAction.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, InputAction.InputTag);
			}
			if (HeldFunc)
			{
				BindAction(InputAction.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, InputAction.InputTag);
			}
		}
	}
}