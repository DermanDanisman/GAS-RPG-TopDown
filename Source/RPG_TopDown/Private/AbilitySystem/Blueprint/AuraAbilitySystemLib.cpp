// © 2025 Heathrow (Derman). All rights reserved.This project is the intellectual property of Heathrow (Derman)
// and is protected by copyright law. Unauthorized reproduction, distribution, or use of this material is strictly prohibited.
// Unreal Engine and its associated trademarks are used under license from Epic Games.

#include "AbilitySystem/Blueprint/AuraAbilitySystemLib.h"

#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/TDAttributeSet.h"
#include "UI/HUD/TDHUD.h"
#include "UI/WidgetControllers/TDHUDWidgetController.h"
#include "UI/WidgetControllers/TDAttributeMenuWidgetController.h"
#include "UI/WidgetControllers/GASCoreUIWidgetController.h"

UTDHUDWidgetController* UAuraAbilitySystemLib::GetHUDWidgetController(const UObject* WorldContext)
{
	// Get the local player controller (index 0)
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
	{
		// Cast the HUD to our custom TDHUD type
		if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
		{
			// Get the player state
			APlayerState* PS = PC->GetPlayerState<APlayerState>();
			
			// Resolve the AbilitySystemComponent  
			UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent(PC);
			
			// Resolve the AttributeSet
			UAttributeSet* AS = ResolveAttributeSet(ASC);
			
			// Ensure we have all required references
			if (PC && PS && ASC && AS)
			{
				// Build the widget controller parameters
				const FGASCoreUIWidgetControllerParams WCParams(PC, PS, ASC, AS);
				
				// Get the HUD widget controller from TDHUD
				return TDHUD->GetHUDWidgetController(WCParams);
			}
		}
	}
	
	return nullptr;
}

UTDAttributeMenuWidgetController* UAuraAbilitySystemLib::GetAttributeMenuWidgetController(const UObject* WorldContext)
{
	// Get the local player controller (index 0)
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
	{
		// Cast the HUD to our custom TDHUD type
		if (ATDHUD* TDHUD = Cast<ATDHUD>(PC->GetHUD()))
		{
			// Get the player state
			APlayerState* PS = PC->GetPlayerState<APlayerState>();
			
			// Resolve the AbilitySystemComponent  
			UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent(PC);
			
			// Resolve the AttributeSet
			UAttributeSet* AS = ResolveAttributeSet(ASC);
			
			// Ensure we have all required references
			if (PC && PS && ASC && AS)
			{
				// Build the widget controller parameters
				const FGASCoreUIWidgetControllerParams WCParams(PC, PS, ASC, AS);
				
				// Get the Attribute Menu widget controller from TDHUD
				return TDHUD->GetAttributeMenuWidgetController(WCParams);
			}
		}
	}
	
	return nullptr;
}

UAbilitySystemComponent* UAuraAbilitySystemLib::ResolveAbilitySystemComponent(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return nullptr;
	}
	
	// First, try to get ASC from PlayerState (preferred approach for GAS)
	if (APlayerState* PS = PlayerController->GetPlayerState<APlayerState>())
	{
		// Try using the AbilitySystemBlueprintLibrary to get ASC from PlayerState
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PS))
		{
			return ASC;
		}
	}
	
	// Fallback: try to get ASC from the controlled pawn
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn))
		{
			return ASC;
		}
	}
	
	return nullptr;
}

UAttributeSet* UAuraAbilitySystemLib::ResolveAttributeSet(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}
	
	// Query the ASC for our specific AttributeSet type
	return const_cast<UAttributeSet*>(AbilitySystemComponent->GetAttributeSet(UTDAttributeSet::StaticClass()));
}