# Enhanced Input → Ability Mapping (Data-Driven) — TD Input Config

Last updated: 2025-09-02

[Note] Use UTDEnhancedInputComponent for one-call binding. See Binding callbacks with UTDEnhancedInputComponent and Ability Input Tags and Activation for the full path from input to activation.

Additions for this project:
- You can remap inputs at runtime by modifying FGameplayAbilitySpec.DynamicAbilityTags (remove one InputTag, add another).
- Don't forget:
  - Project Settings → Input → Default Input Component Class = UTDEnhancedInputComponent
  - Assign UTDInputConfig on your PlayerController BP
  - Tags: InputTag.LMB, InputTag.RMB, InputTag.QuickSlot1..4