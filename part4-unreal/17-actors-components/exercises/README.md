# 17 — Exercises: Actors & Components

Copy solution files into `Source/CLearnGame/CLearn/` to try them.

### Ex 1 — Moving platform ⭐⭐
`ACLMovingPlatform` moves between its start location and `StartLocation + Offset`
(an `FVector` with `meta = (MakeEditWidget = "true")`, which shows a draggable
diamond in the viewport). It travels at a speed in cm/s, pauses `WaitTime`
seconds at each end (use a **timer**, not a Tick countdown), and must be
frame-rate independent. Hint: `FMath::VInterpConstantTo`.
→ `solutions/CLMovingPlatform.h/.cpp`

### Ex 2 — Health pickup that uses the component ⭐
`ACLHealthPickup`: an overlap sphere. When something with a
`UCLHealthComponent` overlaps it (`OtherActor->FindComponentByClass<UCLHealthComponent>()`),
heal it by `HealAmount`, then hide the pickup and **respawn it** after
`RespawnTime` seconds. Don't destroy it: disable collision and visibility,
then re-enable them with a timer.
→ `solutions/CLHealthPickup.h/.cpp`

### Ex 3 — A runtime component ⭐⭐
Write `UCLBlinkComponent`, a plain `UActorComponent` (no transform needed) that
toggles the owner's visibility (`GetOwner()->SetActorHiddenInGame`) every
`Interval` seconds for `Duration` seconds, then stops. Add it **at runtime**
when the player takes damage. Hints: `NewObject<UCLBlinkComponent>(Owner)`,
`RegisterComponent()`, and `DestroyComponent()` when done. This gives you
invulnerability-frame feedback without any Blueprint.
→ `solutions/CLBlinkComponent.h/.cpp`
