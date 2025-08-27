// Copyright:
// © 2025 Heathrow (Derman). All rights reserved.
//
// Implementation notes:
// - This controller assumes AttributeSet is of type UCoreAttributeSet (cast-checked)
// - Binds to ASC delegates using AddLambda with a capture of `this`
//   Ensure this controller's lifetime outlives the ASC bindings, or use weak captures/handles
// - For Effect Asset Tags, we rely on UCoreAbilitySystemComponent to broadcast OnEffectAssetTags
// - UI message routing expects a DataTable with rows keyed by tag FNames (Tag.GetTagName())

#include "GASCore/Public/UI/WidgetControllers/CoreHUDWidgetController.h"

#include "GASCore/Public/Attributes/CoreAttributeSet.h"
#include "GASCore/Public/Components/CoreAbilitySystemComponent.h"
