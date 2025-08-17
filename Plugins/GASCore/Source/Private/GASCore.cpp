// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASCore.h"

#include "Utilities/GASCoreLogging.h"

#define LOCTEXT_NAMESPACE "FGASCoreModule"

void FGASCoreModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	GASCORE_LOG_LOG(TEXT("GASCore module has started!"));
    
	// Register gameplay tags if needed
	// This would be a good place to register custom gameplay tags used by your plugin
}

void FGASCoreModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	GASCORE_LOG_LOG(TEXT("GASCore module has been shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGASCoreModule, GASCore)