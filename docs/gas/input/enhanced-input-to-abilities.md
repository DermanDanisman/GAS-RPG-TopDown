# Enhanced Input → Ability Mapping (Data-Driven) — TD Input Config

Last updated: 2025-09-02

Use UTDEnhancedInputComponent for one-call binding. See the centralization guide for the canonical input tags.

Reminders:
- Project Settings → Input → Default Input Component Class = UTDEnhancedInputComponent
- Assign on ATDPlayerController BP/defaults:
  - GASInputMappingContext (UInputMappingContext)
  - MoveAction (UInputAction)
  - InputConfig (UTDInputConfig)
- Input tags from FTDGameplayTags: InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4
- Runtime remapping: edit FGameplayAbilitySpec::GetDynamicSpecSourceTags() to swap InputTag.* at runtime