// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "Interfaces/GASCoreCombatInterface.h"

// Add default functionality here for any ICombatInterface functions that are not pure virtual.
int32 IGASCoreCombatInterface::GetActorLevel()
{
	return 0;
}

FVector IGASCoreCombatInterface::GetAbilitySpawnLocation()
{
	return FVector();
}
