// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighlightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UHighlightInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Implement this interface on any actor that should react to highlight events.
 */
class HIGHLIGHTACTOR_API IHighlightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	/** Called when the actor should be highlighted. */
	virtual void HighlightActor() = 0;
	/** Called when the actor should remove highlight. */
	virtual void UnHighlightActor() = 0;
};
