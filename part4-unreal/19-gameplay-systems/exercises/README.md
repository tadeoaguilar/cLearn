# 19 — Exercises: Gameplay Systems

### Ex 1 — Lever that controls other actors ⭐⭐
`ACLLever` implements `ICLInteractable`. It has
`UPROPERTY(EditInstanceOnly) TArray<TObjectPtr<AActor>> Targets;` (pick them in
the level with the eyedropper). Interacting toggles the lever, and calls
`Execute_Interact` on every target that implements the interface, so a lever can
open a door. Expose a `FCLOnLeverToggled` dynamic multicast delegate.
→ `solutions/CLLever.h/.cpp`

### Ex 2 — Checkpoints ⭐⭐
`ACLCheckpoint` is an overlap box. When the player touches it, it becomes the
**respawn point** for that player. Store it in `ACLPlayerState`, or in a
subclass of the GameMode. Make respawns use the latest checkpoint's transform:
override `RestartPlayer` and call `RestartPlayerAtTransform`. (Alternatives:
override `ChoosePlayerStart_Implementation`/`FindPlayerStart`, which have to
return an actor.) Only the newest checkpoint should be active (give it visual feedback
with a `BlueprintImplementableEvent`).
→ `solutions/CLCheckpoint.h/.cpp` + `solutions/CLCheckpointGameMode.h/.cpp`

### Ex 3 — Turret: timers + traces + damage ⭐⭐⭐
`ACLTurret` rotates its head toward the player when the player is within
`Range`, but only with **line of sight** (trace on `ECC_Visibility`). Every
`FireInterval` seconds it hits the player with `ApplyPointDamage`. Draw debug
lines while it's shooting. Use a timer for firing, and Tick only for the smooth
rotation (`FMath::RInterpTo`).
→ `solutions/CLTurret.h/.cpp`
