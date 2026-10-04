# 19 — Gameplay Systems: Damage, Delegates, Interfaces & Game Rules

> Goal: connect independent pieces into a game. We use the damage pipeline,
> **delegates** (the observer pattern), **interfaces**, **PlayerState** for
> data that survives death, and a **GameMode** that owns the rules.

## 1. Delegates: Unreal's observer pattern

A delegate is a type-safe callback, like the `std::function` callbacks of
ch. 2/10, but able to bind **safely to UObjects**: if the listener is
garbage-collected, the binding just stops firing.

| Kind | Declare | Listeners | Blueprint? |
|------|---------|-----------|------------|
| single | `DECLARE_DELEGATE_OneParam(FOnX, int32)` | 1 | ❌ |
| multicast | `DECLARE_MULTICAST_DELEGATE_OneParam(FOnX, int32)` | many | ❌ |
| dynamic | `DECLARE_DYNAMIC_DELEGATE_OneParam(FOnX, int32, Value)` | 1 | ✅ (serializable, slower) |
| dynamic multicast | `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnX, int32, Value)` | many | ✅ `BlueprintAssignable` |

```cpp
// CLHealthComponent.h — the subject declares and owns the event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCLOnHealthChanged, UCLHealthComponent*, HealthComponent, float, NewHealth, float, Delta);
UPROPERTY(BlueprintAssignable) FCLOnHealthChanged OnHealthChanged;
OnHealthChanged.Broadcast(this, Health, Health - Old);      // notify everyone

// CLHUDWidget.cpp — an observer subscribes; the handler MUST be a UFUNCTION for dynamic delegates
Health->OnHealthChanged.AddDynamic(this, &UCLHUDWidget::HandleHealthChanged);
Health->OnHealthChanged.RemoveAll(this);                    // unsubscribe (e.g. when rebinding)
```
**Why this matters:** `UCLHealthComponent` doesn't know the HUD, the GameMode
or the character exist. The character subscribes to `OnDeath`, and the HUD to
`OnHealthChanged`. Add a damage-sound component tomorrow and nothing else
changes. That is low coupling.

Data flow in Orb Runner:
```
DamageZone ─ApplyDamage─► Actor.OnTakeAnyDamage ─► HealthComponent ─OnHealthChanged─► HUD bar
                                                         └─OnDeath─► Character ─► GameMode.NotifyPlayerDied ─► respawn timer
Orb overlap ─► PlayerState.AddOrbs ─OnOrbsChanged─► HUD text
          └──► GameMode.NotifyOrbCollected ─► (all orbs?) EndRun ─OnRunEnded─► HUD message
                                                         ├─► SaveSubsystem.RecordRun (ch. 20)
                                                         └─► QuestSubsystem.ReportEvent (ch. 20)
```

## 2. The damage pipeline

```cpp
UGameplayStatics::ApplyDamage(Victim, 10.f, InstigatorController, DamageCauser, UDamageType::StaticClass());
```
This calls `Victim->TakeDamage(...)`, which broadcasts `Victim->OnTakeAnyDamage`.
`UCLHealthComponent` subscribes to its owner's `OnTakeAnyDamage` in `BeginPlay`,
so **anything** can hurt **anything** with a health component, without either
side knowing the other's class. Variants: `ApplyPointDamage` (with a hit
location and direction) and `ApplyRadialDamage` (explosions with falloff).
Custom `UDamageType` subclasses carry the damage *kind* (fire, fall, poison).

`ACLDamageZone` uses a **looping timer** while pawns are inside. Each tick, it
asks the physics scene who's overlapping (`GetOverlappingActors`) instead of
keeping its own list, so there's no bookkeeping that can drift out of sync.

## 3. Interfaces

The character wants to "interact" with doors, levers and NPCs, which share no
base class. An **interface** describes a capability:

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UCLInteractable : public UInterface { GENERATED_BODY() };     // reflection boilerplate

class CLEARNGAME_API ICLInteractable                                 // the real interface
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FText GetInteractionPrompt() const;
};

class ACLDoor : public AActor, public ICLInteractable {              // implement in C++...
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual FText GetInteractionPrompt_Implementation() const override;
};
// ...or in Blueprint: Class Settings → Interfaces → Add → CLInteractable
```
**Calling** it must work for both C++ and Blueprint implementers:
```cpp
if (Target->Implements<UCLInteractable>())                 // note the U-class here
    ICLInteractable::Execute_Interact(Target, this);       // and Execute_ here
```
❌ `Cast<ICLInteractable>(Target)->Interact(...)` fails for Blueprint-only implementations.

## 4. PlayerState: data that survives death

When the character dies, the pawn is destroyed. Anything stored on it is lost.
`ACLPlayerState` holds the orb count and deaths, so they persist across respawns,
and in multiplayer it's replicated to every client. (For multiplayer, add
`UPROPERTY(Replicated)` / `ReplicatedUsing` and `GetLifetimeReplicatedProps`.
That's a topic for further study.)

```cpp
if (ACLPlayerState* PS = Pawn->GetPlayerState<ACLPlayerState>()) PS->AddOrbs(Value);
```

## 5. GameMode: the referee

`ACLGameMode` (parent `AGameModeBase`):
- sets the default classes (`DefaultPawnClass`, `PlayerControllerClass`, `PlayerStateClass`),
- counts the orbs in the level at `BeginPlay` with `TActorIterator`,
- tracks run time with `GetWorld()->GetTimeSeconds()` and a time-limit timer,
- on death: applies an orb penalty, then **respawns** after a delay with a
  timer delegate that carries a payload:
  ```cpp
  GetWorldTimerManager().SetTimer(Handle,
      FTimerDelegate::CreateUObject(this, &ACLGameMode::Respawn, TWeakObjectPtr<AController>(Victim)),
      RespawnDelay, false);
  ```
  `RestartPlayer(Controller)` spawns a new pawn at a `PlayerStart` and possesses it,
- on all orbs collected or time up: `EndRun`, which stops the clock, records the
  run (ch. 20), reports the quest event (ch. 20), and broadcasts `OnRunEnded`.

The GameMode **exists only on the server**. Clients never see it. Our
single-player game reads it from the HUD for simplicity. In multiplayer you'd
mirror the needed data in a `AGameStateBase` subclass.

## 6. Death, ragdoll and respawn (in `ACLCharacter::HandleDeath`)

1. Disable movement and capsule collision.
2. Switch the mesh to the `Ragdoll` profile and `SetSimulatePhysics(true)`.
3. Tell the GameMode (it schedules the respawn).
4. `DetachFromControllerPendingDestroy()`: the controller is now free.
5. `SetLifeSpan(5.f)`: the body disappears later.
6. `OnPossess` on the controller rebinds the HUD to the new pawn (ch. 20).

## Walkthrough of this chapter's classes

| File | Concepts |
|------|----------|
| `CLHealthComponent` (revisited) | damage pipeline, invulnerability window, `OnDeath` |
| `CLDamageZone` | overlap begin/end, looping timer, `ApplyDamage` |
| `CLInteractable.h` | `UINTERFACE`, `BlueprintNativeEvent`, `Execute_` calls |
| `CLDoor` | implementing an interface, `FInterpTo` animation with Tick enabled only while moving, `LOCTEXT`/`FText::Format` |
| `CLPlayerState` | persistent per-player data + delegate |
| `CLGameMode` | rules, `TActorIterator`, timers with payloads, `RestartPlayer`, run end |
| `CLCharacter::HandleDeath` | ragdoll + respawn handshake |

## In the editor

1. **BP_DamageZone** (parent `CLDamageZone`): add a red translucent plane as a
   visual, and place it on the floor.
2. **BP_Door** (parent `CLDoor`): set `Panel` to a cube scaled (0.2, 2, 3). Move
   the `Panel` so its edge sits at the `Hinge`, and set `RequiredOrbs = 3` on
   one instance.
3. Make sure the level has a **PlayerStart**: it's where respawns happen.
4. Play: walk into the damage zone, die, respawn, collect orbs, and open the door.

Next: [exercises](exercises/README.md)
