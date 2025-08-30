// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "TDAssetManager.h"
#include "TDGameplayTags.h" // FTDGameplayTags::InitializeNativeGameplayTags

UTDAssetManager& UTDAssetManager::Get()
{
	check(GEngine);

	UTDAssetManager* TDAssetManager = Cast<UTDAssetManager>(GEngine->AssetManager);
	check(TDAssetManager); // Ensure asset manager is of expected type.

	return *TDAssetManager;
}

void UTDAssetManager::StartInitialLoading()
{
	// Maintain base asset manager initialization.
	Super::StartInitialLoading();

	// Register all native gameplay tags used by the project before gameplay begins.
	FTDGameplayTags::InitializeNativeGameplayTags();
}
