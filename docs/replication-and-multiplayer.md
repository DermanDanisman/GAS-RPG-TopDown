# Replication and Multiplayer

Last updated: 2025-08-17

## Roles

- Server: authoritative; runs game logic; sends state to clients
- Clients: local simulation + prediction; receive server updates

Replication:
- Variables replicate server -> client (one-way)
- Clients use RPCs to request actions (not covered here in depth)

## Class Presence

- GameMode: server only
- PlayerController: server + owning client
- PlayerState: server + all clients
- Pawns/Characters: server + all clients
- HUD/Widgets: local to each client; not on dedicated server

## ASC Replication Mode

`UAbilitySystemComponent::SetReplicationMode(EGameplayEffectReplicationMode)`

- Full: replicate full effect info to all (rare; single-player)
- Mixed: minimal to simulated proxies; full to owner/autonomous
- Minimal: minimal effect info; rely on tags/cues replicating

Recommended:
- Player ASC (on PlayerState): Mixed
- AI ASC (on Enemy Character): Minimal

## Net Update Frequency

- PlayerState often set to ~100 for responsive ASC/Attribute replication

## Prediction

- Effects can predict changes client-side (smoother UX)
- Server validates and rolls back invalid changes