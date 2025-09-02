// © 2025 Heathrow (Derman). All rights reserved.
// This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GASCoreAbilityInputConfig.h"
#include "GASCoreEnhancedInputComponent.generated.h"

/**
 * Custom input component that binds ability input actions from a data-driven config.
 * Provides template-based binding for input delegates using tags.
 *
 * Modular notes:
 * - This component is plugin-agnostic regarding the game module; it only depends on EnhancedInput and GameplayTags.
 * - Your game can subclass PlayerController and use this component directly.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASCORE_API UGASCoreEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/** Default constructor. Disables tick. */
	UGASCoreEnhancedInputComponent();

	/**
	 * Binds all ability input actions defined in the input config to the given user class' handler functions.
	 *
	 * @param InputConfig Data asset defining input actions and tags.
	 * @param Object The object (usually a player controller) to bind delegates to.
	 * @param PressedFunc Handler for "Pressed/Started" (may be nullptr if not used).
	 * @param ReleasedFunc Handler for "Released/Completed" (may be nullptr if not used).
	 * @param HeldFunc Handler for "Held/Triggered" (may be nullptr if not used).
	 *
	 * Note:
	 * - Use function pointer null checks, not .IsValid() (which is not valid for raw pointers).
	 * - Template kept in header to avoid linker issues with template instantiation across modules.
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityInputActions(const UGASCoreAbilityInputConfig* InputConfig, UserClass* Object,
		PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UGASCoreEnhancedInputComponent::BindAbilityInputActions(const UGASCoreAbilityInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	for (const FGASCoreAbilityInputAction& InputAction : InputConfig->AbilityInputActions)
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