// © 2025 Heathrow (Derman). All rights reserved. This project is the intellectual property of Heathrow (Derman) and is protected by copyright law.
// Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#pragma once

// ===== Engine Includes =====
#include "CoreMinimal.h"
#include "Engine/AssetManager.h"

#include "TDAssetManager.generated.h"

/**
 * UTDAssetManager
 *
 * Central asset manager for the project.
 * - Serves as a bootstrap point to initialize native gameplay tags and other global systems.
 */
UCLASS()
class RPG_TOPDOWN_API UTDAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	/** Returns a reference to the project's AssetManager (cast from GEngine->AssetManager). */
	static UTDAssetManager& Get();

protected:
	/** Called during engine startup to initialize project-wide systems. */
	virtual void StartInitialLoading() override;
};

