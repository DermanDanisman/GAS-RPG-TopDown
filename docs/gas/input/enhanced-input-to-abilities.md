# Enhanced Input → Ability Mapping (Data-Driven) — TD Input Config

Last updated: 2025-09-02

[Note] Use UTDEnhancedInputComponent for one-call binding. See Binding callbacks with UTDEnhancedInputComponent and Ability Input Tags and Activation for the full path from input to activation.

Reminders for this project:
- Project Settings → Input → Default Input Component Class = UTDEnhancedInputComponent
- Assign on ATDPlayerController BP/defaults:
  - GASInputMappingContext (UInputMappingContext)
  - MoveAction (UInputAction)
  - InputConfig (UTDInputConfig)
- Input tags: InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4 (defined in your centralized tags)
- You can remap inputs at runtime by editing FGameplayAbilitySpec::GetDynamicSpecSourceTags()