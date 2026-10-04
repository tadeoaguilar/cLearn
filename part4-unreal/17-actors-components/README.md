# 17 — Actors & Components

> Goal: understand Unreal's object model for things in the world. That covers
> **Actors** (entities), **Components** (reusable behavior and visuals), their
> lifecycle, Tick vs timers, spawning and destruction, and exposing C++ to the editor.

## 1. Actors and components

```
ACLOrb (AActor)                         "an entity in the level"
 └─ Collision : USphereComponent  (root) has a transform; defines where the actor is
     └─ Mesh : UStaticMeshComponent      attached child; relative transform
```
- An **Actor** is anything placed or spawned in a level: characters, pickups,
  lights, triggers. Actors have no transform of their own. They use their
  **RootComponent**'s.
- **Components** add capabilities:
  - `UActorComponent`: logic only, with no transform (e.g. `UCLHealthComponent`).
  - `USceneComponent`: has a transform and can be attached in a hierarchy.
  - `UPrimitiveComponent`: renderable and/or collidable (meshes, shapes).
- **Composition over inheritance** (ch. 4 again): instead of `ACollectibleHealthyRotatingThing`,
  build actors out of components. `UCLHealthComponent` works on *any* actor: the
  player, enemies, destructible barrels.

### Creating components (constructor only)
```cpp
Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision")); // name must be unique per actor
RootComponent = Collision;
Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
Mesh->SetupAttachment(Collision);
```
At runtime, use `NewObject<UMyComponent>(this)` + `RegisterComponent()` +
`AttachToComponent(...)`.

## 2. The actor lifecycle

`CLLifecycleActor` logs each step. Place one in a level, press Play then Stop,
and read the log:

```
Constructor ─► OnConstruction (editor, on every edit) ─► PostInitializeComponents
   ─► BeginPlay ─► Tick, Tick, Tick ... ─► EndPlay(reason) ─► Destroyed ─► (GC frees memory)
```
| Function | Use it for | Don't |
|----------|-----------|-------|
| Constructor | create components, set **defaults** | access the world, other actors, or gameplay state: it also runs for the Class Default Object |
| `OnConstruction` | editor-time procedural setup (C++ "construction script") | gameplay |
| `BeginPlay` | gameplay init: bind delegates, start timers, find other actors | heavy work every spawn |
| `Tick(DeltaSeconds)` | per-frame continuous behavior | things that could be events or timers |
| `EndPlay` | cleanup: clear timers, unbind from long-lived objects | assume `Destroy()` was the reason |

**Always call `Super::X()`** in overrides. The base classes do essential work there.

## 3. Tick vs timers vs events

| Mechanism | Cost | Example |
|-----------|------|---------|
| Tick (every frame) | per-frame, per-actor | `ACLOrb` spin/bob: genuinely continuous motion |
| Timer (`FTimerManager`) | only when it fires | `ACLSpawner` every 3 s; `ACLCharacter` focus trace 10×/s |
| Event/delegate | zero until something happens | overlap → collect; health changed → HUD |

Prefer **events, then timers, then Tick**. Disable ticking you don't need:
`PrimaryActorTick.bCanEverTick = false;`, or turn it on only while animating,
as `ACLDoor` does with `SetActorTickEnabled`.

**Frame-rate independence**: multiply by `DeltaSeconds`.
`AddActorLocalRotation(FRotator(0, RotationSpeed * DeltaSeconds, 0))` spins at
the same speed at 30 and 144 FPS.

```cpp
FTimerHandle Handle;                                                   // remember it to cancel
GetWorldTimerManager().SetTimer(Handle, this, &ACLSpawner::SpawnOne, Interval, /*bLoop=*/true);
GetWorldTimerManager().ClearTimer(Handle);
GetWorldTimerManager().SetTimer(H, FTimerDelegate::CreateLambda([]{ ... }), 2.f, false); // lambda
```

## 4. Exposing C++ to designers

```cpp
UPROPERTY(EditAnywhere, Category = "Orb|Motion", meta = (Units = "deg"))
float RotationSpeed = 90.f;

UFUNCTION(BlueprintImplementableEvent, Category = "Orb")
void OnCollected(AActor* Collector);   // NO C++ body: Blueprint implements it (VFX, sounds)

UFUNCTION(BlueprintNativeEvent)        // C++ default in X_Implementation(), Blueprint may override
void OnHit(); void OnHit_Implementation();
```
Designers create **BP_Orb** (parent class `CLOrb`), pick a mesh and material
for the `Mesh` component, tweak `RotationSpeed`, and implement `OnCollected`
with a Niagara burst. No C++ recompile is needed.

## 5. Collision & overlap events

Every primitive has a **collision profile**: which channels it blocks, overlaps
or ignores. We use `OverlapAllDynamic` for pickups and zones.
```cpp
Collision->SetGenerateOverlapEvents(true);
Collision->OnComponentBeginOverlap.AddDynamic(this, &ACLOrb::HandleBeginOverlap);  // in BeginPlay
UFUNCTION() void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
```
- `AddDynamic` binds by **function name** through reflection, so the handler
  must be a `UFUNCTION()` with **exactly** the delegate's parameter list.
- Both sides need overlap events enabled, and must overlap each other's channel.
  "My overlap never fires" is almost always a collision-settings problem. Turn
  on *Show → Collision* in the viewport.

## 6. Spawning and destroying

```cpp
FActorSpawnParameters Params;
Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
AActor* A = GetWorld()->SpawnActor<AActor>(ActorClass, Location, Rotation, Params);  // may return nullptr!
A->Destroy();             // removed at end of frame
A->SetLifeSpan(5.f);      // destroy in 5 s
```
`TSubclassOf<AActor> ActorClass` is a type-safe class reference. In the editor
you pick `BP_Orb` from a dropdown filtered to Actor subclasses.

`ACLSpawner` keeps `TArray<TWeakObjectPtr<AActor>>` to count living spawns.
Weak pointers become invalid when the actor dies, so we don't need to track
destruction manually (`RemoveAll(!IsValid())`).

## 7. Finding other actors

```cpp
for (TActorIterator<ACLOrb> It(GetWorld()); It; ++It) { ... }          // all of a type (EngineUtils.h)
UGameplayStatics::GetAllActorsOfClass(this, ACLOrb::StaticClass(), Out);
UGameplayStatics::GetPlayerPawn(this, 0);
Cast<ACLCharacter>(OtherActor);                                         // safe downcast, nullptr on mismatch
```
Iterating every actor is O(n). Do it once (in BeginPlay) and cache the result,
or use events instead.

## Walkthrough of this chapter's classes

| File | Concepts |
|------|----------|
| `CLLifecycleActor.h/.cpp` | full lifecycle with logs, `SetLifeSpan`, `EEndPlayReason` |
| `CLOrb.h/.cpp` | components, root/attachment, Tick with `DeltaSeconds`, overlap events, `BlueprintImplementableEvent`, `Destroy` |
| `CLHealthComponent.h/.cpp` | a reusable `UActorComponent`, dynamic multicast delegates, binding to the owner's `OnTakeAnyDamage` |
| `CLSpawner.h/.cpp` | `TSubclassOf`, looping timers, `SpawnActor`, weak pointers, `OnDestroyed` delegate, cleanup in `EndPlay` |

## In the editor

1. **BP_Orb**: Content Browser → right-click → Blueprint Class → All Classes → `CLOrb`.
   Set `Mesh` to a sphere (Engine content: `/Engine/BasicShapes/Sphere`) with a
   glowing material, and scale it down to ~0.4.
2. Drop a few BP_Orbs and one `CLLifecycleActor` into the level, and press Play.
3. **Spawner**: place a `CLSpawner`, set `ActorClass = BP_Orb`, and scale its
   box over the floor.
4. Try *Window → Output Log* with `LogCLearn` filtering, and run
   `Log LogCLearn Verbose` in the console (`~`) to see verbose lines.

Next: [exercises](exercises/README.md)
