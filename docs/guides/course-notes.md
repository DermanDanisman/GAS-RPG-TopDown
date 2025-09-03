# Course Notes and Mapping

Last updated: 2024-12-19

This file maps course transcripts to implemented patterns in this project.

- widget_controller_broadcast.cpp — Base Widget Controller, BroadcastInitialValues, dynamic multicast delegates for Health/MaxHealth
- gameplay_ability_intro.cpp / gas_system_overview.cpp — GAS fundamentals: ASC, Attribute Set, Abilities, Tasks, Effects, Cues, Tags
- aura_player_state.cpp — PlayerState creation, NetUpdateFrequency=100, set as GameMode PlayerState
- aura_ability_system.cpp — Creating ASC/AttributeSet classes; Build.cs modules (Abilities/Tags/Tasks)
- multiplayer_gas_system.cpp / replication — Server/client roles; what replicates where; HUD local-only
- ability_system_replication.cpp — Replication modes: Mixed (player), Minimal (AI)
- ability_system_overview.cpp / attribute_accessors.cpp — Attribute design; Accessor macros; Init vs Set; showdebug abilitysystem
- aura_effect_actor.cpp — Effect Actor; overlap bindings; accessing ASC via AbilitySystemInterface; early direct attribute mutation (and why to avoid)
- gameplay_effects_overview.cpp / gameplay_effect_potion.cpp — Using Gameplay Effects (Instant); Blueprint vs C++ apply paths
- gameplay_effect_duration.cpp / periodic notes — Duration effects; Periodic; Current vs Base value; execute on application; interval design
- gameplay_effect_stacking.cpp — Stacking types and policies
- infinite_gameplay_effect.cpp — Infinite effects; removal policies; area-of-effect fire example
- gameplay_tags_intro.cpp / effect_applied_callback.cpp / effect_asset_tags_delegate.cpp — Gameplay tags; ASC delegate binding; broadcasting asset tags to Widget Controller