# TD Asset Manager (Startup and Native Tags Initialization)

Last updated: 2024-12-19

## Goal

Create a `UTDAssetManager` subclass that extends Unreal Engine's `UAssetManager` to handle startup initialization tasks, specifically calling `FTDGameplayTags::InitializeNativeGameplayTags()` during the engine's asset loading phase.

## Why Use Asset Manager for Initialization

### The Problem with Module Startup

While you could initialize native gameplay tags in your game module's `StartupModule()`, the Asset Manager approach provides better guarantees:

```cpp
// Less reliable - module startup timing can vary
void FRPGModule::StartupModule()
{
    // May run before gameplay tag system is ready
    FTDGameplayTags::InitializeNativeGameplayTags();
}
```

### Asset Manager Benefits

- **Guaranteed Timing**: Runs after core systems are initialized
- **Engine Integration**: Part of Unreal's standard asset loading pipeline
- **Project-Specific**: Cleanly separates game logic from engine concerns
- **Extensible**: Easy to add more startup tasks as needed

## UTDAssetManager Implementation

### Header Definition (AuraAssetManager.h)

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * UTDAssetManager
 * 
 * Custom Asset Manager for the Aura project that handles startup initialization
 * tasks including native gameplay tag registration.
 * 
 * Configure this class as the project's asset manager in DefaultEngine.ini
 * to ensure proper initialization order during engine startup.
 */
UCLASS()
class AURA_API UTDAssetManager : public UAssetManager
{
    GENERATED_BODY()

public:
    /**
     * Get the project's asset manager instance, cast to UTDAssetManager.
     * 
     * @return Pointer to the Aura asset manager, or nullptr if not properly configured
     */
    static UTDAssetManager* Get();

protected:
    /**
     * Override StartInitialLoading to perform custom startup initialization.
     * 
     * This method is called by the engine during the asset loading phase,
     * after core systems are initialized but before game content is fully loaded.
     * 
     * Perfect timing for registering native gameplay tags and other startup tasks.
     */
    virtual void StartInitialLoading() override;
};
```

### Implementation (AuraAssetManager.cpp)

```cpp
#include "AuraAssetManager.h"
#include "AuraGameplayTags.h"

UTDAssetManager* UTDAssetManager::Get()
{
    // Get the global asset manager and cast to our custom type
    if (UAssetManager* AssetManager = UAssetManager::GetIfValid())
    {
        return Cast<UTDAssetManager>(AssetManager);
    }
    
    return nullptr;
}

void UTDAssetManager::StartInitialLoading()
{
    // Always call the parent implementation first
    Super::StartInitialLoading();
    
    // Initialize our native gameplay tags
    // This happens after core systems are ready but before game content loads
    FTDGameplayTags::InitializeNativeGameplayTags();
    
    // Add other startup initialization here as needed:
    // - Custom asset registry setup
    // - Global game data initialization
    // - Performance profiling setup
    // - etc.
}
```

## Engine Configuration

### DefaultEngine.ini Setup

Configure the engine to use your custom Asset Manager by adding this to `Config/DefaultEngine.ini`:

```ini
[/Script/Engine.Engine]
AssetManagerClassName="/Script/Aura.AuraAssetManager"
```

**Important Notes:**

- Replace `Aura` with your actual module name if different
- The path format is `/Script/[ModuleName].[ClassName]`
- This setting tells Unreal to instantiate `UTDAssetManager` instead of the default `UAssetManager`
- The change requires an editor restart to take effect

### Verification

To verify the Asset Manager is properly configured:

1. **Check Logs**: Look for initialization messages during startup
2. **Debug Access**: Use `UTDAssetManager::Get()` in code to verify casting works
3. **Tag Availability**: Ensure `FTDGameplayTags::Get()` returns valid tags after initialization

```cpp
// Example verification code
void AMyActor::VerifyAssetManager()
{
    if (UTDAssetManager* AuraAssetManager = UTDAssetManager::Get())
    {
        UE_LOG(LogTemp, Log, TEXT("Aura Asset Manager is active"));
        
        // Test that tags were initialized
        const FTDGameplayTags& GameplayTags = FTDGameplayTags::Get();
        if (FTDGameplayTags::Get().Attributes_Primary_Strength.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("Native gameplay tags initialized successfully"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Aura Asset Manager not found - check DefaultEngine.ini configuration"));
    }
}
```

## Startup Sequence

### Initialization Order

The startup sequence ensures proper timing:

1. **Engine Core**: Unreal initializes core systems
2. **Asset Manager Created**: Engine creates `UTDAssetManager` instance
3. **StartInitialLoading Called**: Our override executes
4. **Super Called**: Base asset manager initialization
5. **Tags Initialized**: `FTDGameplayTags::InitializeNativeGameplayTags()` executes
6. **Asset Loading**: Engine continues with asset discovery and loading
7. **Game Ready**: Game systems can now safely use centralized tags

### Thread Safety

- `StartInitialLoading` runs on the main thread
- Native gameplay tag registration is thread-safe
- Tags are available immediately after initialization completes

## Advanced Usage

### Additional Startup Tasks

The Asset Manager is an ideal place for other startup initialization:

```cpp
void UTDAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    // Initialize native gameplay tags first
    FTDGameplayTags::InitializeNativeGameplayTags();
    
    // Example: Initialize global data assets
    LoadGlobalDataAssets();
    
    // Example: Setup custom asset registry filters
    SetupAssetRegistryFilters();
    
    // Example: Initialize analytics or telemetry
    InitializeAnalytics();
}
```

### Asset Loading Customization

You can also override other Asset Manager methods for advanced asset handling:

```cpp
// Custom primary asset type handling
virtual UPrimaryDataAsset* LoadPrimaryAsset(const FPrimaryAssetId& PrimaryAssetId, 
    const TArray<FName>& LoadBundles = TArray<FName>()) override;

// Custom asset discovery
virtual void StartInitialLoading() override;

// Asset streaming customization  
virtual int32 GetStreamingPriority(const FSoftObjectPath& AssetPath) const override;
```

### Development vs Shipping

Consider different behavior for development builds:

```cpp
void UTDAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    FTDGameplayTags::InitializeNativeGameplayTags();
    
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
    // Development-only initialization
    EnableDetailedTagLogging();
    ValidateAllGameplayTags();
#endif
    
#if UE_BUILD_SHIPPING
    // Shipping-only optimizations
    PreloadCriticalAssets();
#endif
}
```

## Troubleshooting

### Common Issues

**Asset Manager Not Loading:**
- Check `DefaultEngine.ini` syntax and module name
- Verify the class is properly exported (`AURA_API`)
- Ensure the module compiles successfully
- Restart the editor after configuration changes

**Tags Not Initialized:**
- Add logging to verify `StartInitialLoading` is called
- Check for exceptions during tag registration
- Verify `FTDGameplayTags::InitializeNativeGameplayTags()` implementation

**Blueprint Integration Issues:**
- Ensure tags are marked `BlueprintReadOnly`
- Check that `USTRUCT(BlueprintType)` is present
- Verify proper `GENERATED_BODY()` usage

### Debug Logging

Add logging to track initialization:

```cpp
void UTDAssetManager::StartInitialLoading()
{
    UE_LOG(LogTemp, Log, TEXT("UTDAssetManager::StartInitialLoading - Begin"));
    
    Super::StartInitialLoading();
    
    UE_LOG(LogTemp, Log, TEXT("Initializing native gameplay tags..."));
    FTDGameplayTags::InitializeNativeGameplayTags();
    UE_LOG(LogTemp, Log, TEXT("Native gameplay tags initialized"));
    
    UE_LOG(LogTemp, Log, TEXT("UTDAssetManager::StartInitialLoading - Complete"));
}
```

## Best Practices

### Single Responsibility

Keep the Asset Manager focused on startup initialization:
- Register native gameplay tags
- Load critical global assets
- Setup core game systems

Avoid game-specific logic that belongs in other systems.

### Error Handling

Add robust error handling for production builds:

```cpp
void UTDAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    
    try
    {
        FTDGameplayTags::InitializeNativeGameplayTags();
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize gameplay tags: %s"), 
            *FString(e.what()));
        // Handle gracefully or abort initialization
    }
}
```

### Configuration Management

Consider exposing configuration options:

```cpp
UCLASS()
class AURA_API UTDAssetManager : public UAssetManager
{
    // ... other code ...

protected:
    /** Whether to enable detailed logging during initialization */
    UPROPERTY(Config, Category = "Aura Asset Manager")
    bool bEnableDetailedLogging = false;
    
    /** List of critical assets to preload */
    UPROPERTY(Config, Category = "Aura Asset Manager")
    TArray<TSoftObjectPtr<UObject>> CriticalAssets;
};
```

## Related Documentation

- [Aura Gameplay Tags](aura-gameplay-tags.md) - The singleton system initialized by this Asset Manager
- [Gameplay Tags Centralization](gameplay-tags-centralization.md) - Broader context on tag management strategy