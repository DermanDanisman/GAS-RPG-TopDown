// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited. Unreal Engine and its associated trademarks are used under license from Epic Games.


#include "TDAssetManager.h"
#include "TDGameplayTags.h"

UTDAssetManager& UTDAssetManager::Get()
{
	check(GEngine);
	
	UTDAssetManager* TDAssetManager = Cast<UTDAssetManager>(GEngine->AssetManager);
	return *TDAssetManager;
}

void UTDAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FTDGameplayTags::InitializeNativeGameplayTags();
}
