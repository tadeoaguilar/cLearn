# 16 — Unreal C++ Primer

> Goal: understand how an Unreal C++ project is structured and built, and the
> rules that make Unreal C++ different from the standard C++ of parts 1–3.
> These are reflection, garbage collection, UE types, naming conventions and
> error handling without exceptions.

## 1. Engine architecture in one page

```
 ┌──────────────── Your game module (CLearnGame) ────────────────┐
 │  Actors, Components, GameMode, Widgets, SaveGames   (C++/BP)  │
 └───────────────────────────────┬───────────────────────────────┘
                                 │ uses
 ┌───────────── Engine modules ──┴───────────────────────────────┐
 │ Engine (Actors, World, Physics), UMG (UI), EnhancedInput,     │
 │ HTTP, Json, Niagara, Chaos, Renderer, Audio ...               │
 ├───────────────────────────────────────────────────────────────┤
 │ CoreUObject: UObject, reflection, garbage collector, serialization │
 ├───────────────────────────────────────────────────────────────┤
 │ Core: FString, TArray, TMap, math, logging, threading          │
 └────────────────────────────────────────────────────────────────┘
```
- The engine is split into **modules** (like the libraries from ch. 12). Your
  game is a module too, and `*.Build.cs` lists its dependencies.
- The **World** contains **Levels**, which contain **Actors** (things placed in
  the world), which contain **Components** (reusable behavior and visuals).
- The **game thread** runs gameplay: `Tick`, input, `BeginPlay`. Rendering,
  audio and physics run on other threads. Gameplay code should stay on the game
  thread unless you know what you're doing.

## 2. Project layout

```
CLearnGame/
├── CLearnGame.uproject           JSON: engine version, modules, plugins
├── Config/                       .ini settings (DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini)
├── Content/                      .uasset files: maps, meshes, Blueprints (binary, use Git LFS)
├── Source/
│   ├── CLearnGame.Target.cs      what to build (Game)
│   ├── CLearnGameEditor.Target.cs  (Editor)
│   └── CLearnGame/               the module
│       ├── CLearnGame.Build.cs   module dependencies
│       ├── CLearnGame.h/.cpp     IMPLEMENT_PRIMARY_GAME_MODULE
│       └── CLearn/               our classes
├── Binaries/ Intermediate/ Saved/ DerivedDataCache/   generated: never commit
```

## 3. The build pipeline

```
  .h with UCLASS()  ──► UnrealHeaderTool (UHT) ──► *.generated.h / *.gen.cpp   (reflection code)
         │                                                   │
         └───────────────► UnrealBuildTool (UBT) ◄───────────┘
                           reads *.Build.cs / *.Target.cs, invokes clang/MSVC
                                     │
                           CLearnGame module (.dylib/.dll in editor builds)
```
- **UHT** parses your headers and generates the reflection glue. That's why
  every reflected header ends its includes with `#include "X.generated.h"`, and
  why the `UCLASS()` macro syntax must be exact.
- **UBT** is Unreal's build system: it plays the role CMake played for us.
- **Live Coding** (Ctrl+Alt+F11 in the editor) recompiles `.cpp` changes while
  the editor runs. Changes to headers and reflected properties need a full
  rebuild with the editor closed.

## 4. Reflection: the macros

```cpp
UCLASS(Blueprintable)                          // class is known to the engine
class CLEARNGAME_API ACLOrb : public AActor    // CLEARNGAME_API = export from the module
{
    GENERATED_BODY()                           // UHT inserts generated members here

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orb", meta = (ClampMin = "0"))
    int32 Value = 1;                           // visible in editor + Blueprints, saved, GC-tracked

    UFUNCTION(BlueprintCallable, Category = "Orb")
    void Collect(AActor* Collector);           // callable from Blueprints, RPC-able, timer/delegate bindable
};
```
| Macro | Purpose |
|-------|---------|
| `UCLASS()` | reflected class (must derive from `UObject`) |
| `USTRUCT()` | reflected value type (no GC, copied by value) |
| `UENUM()` | reflected enum (`enum class X : uint8`) |
| `UPROPERTY()` | reflected member: editor, Blueprints, serialization, **GC reference** |
| `UFUNCTION()` | reflected function: Blueprints, delegates, timers, RPCs, console exec |
| `UINTERFACE()` | reflected interface (ch. 19) |

### Common `UPROPERTY` specifiers
| Specifier | Meaning |
|-----------|---------|
| `EditAnywhere` | editable on the class defaults *and* on placed instances |
| `EditDefaultsOnly` | editable only in the Blueprint class defaults |
| `EditInstanceOnly` | editable only on instances placed in a level |
| `VisibleAnywhere` | shown read-only (typical for components) |
| `BlueprintReadOnly` / `BlueprintReadWrite` | access from Blueprint graphs |
| `BlueprintAssignable` | a multicast delegate Blueprints can bind to |
| `Category = "X\|Y"` | grouping in the Details panel |
| `meta = (ClampMin="0", UIMin="0", AllowPrivateAccess="true")` | editor hints |
| `Transient` | not saved |
| `SaveGame` | included when serializing with the SaveGame flag (ch. 20) |

## 5. UObjects and garbage collection

Unreal has a **garbage collector** for `UObject`s, so you never `delete` one.
- Create them with `NewObject<T>(Outer)`, `CreateDefaultSubobject<T>(TEXT("Name"))`
  (in constructors, for components), or `GetWorld()->SpawnActor<T>(...)` (actors).
- An object stays alive while it's reachable through **`UPROPERTY()` references**
  from the root set: the world, the game instance, and so on.
- ⚠️ **A raw `UObject*` member without `UPROPERTY()` is invisible to the GC.**
  The object can be collected, and your pointer dangles. That's the Unreal
  version of the ch. 3 lifetime bugs.

```cpp
UPROPERTY() TObjectPtr<UHealthComponent> Health;   // ✅ strong, GC-tracked
TWeakObjectPtr<AActor> LastTarget;                 // ✅ weak: becomes null when the actor is destroyed
UHealthComponent* Cached;                          // ❌ as a member: may dangle
```
- Destroying an actor: `Actor->Destroy()` marks it for destruction, and it's
  gone at the end of the frame. Check validity with `IsValid(Actor)`, which is
  false for null **or** pending-kill objects.
- **Non-UObject** C++ classes and structs use normal RAII, `TUniquePtr`, `TSharedPtr`.

## 6. Naming conventions (enforced by UHT for reflected types)

| Prefix | Kind | Example |
|--------|------|---------|
| `A` | Actor subclass | `ACLCharacter` |
| `U` | UObject (non-actor) subclass, including components | `UCLHealthComponent` |
| `F` | struct / plain class | `FCLItem`, `FVector`, `FString` |
| `E` | enum | `ECLRarity` |
| `I` | interface | `ICLInteractable` |
| `T` | template | `TArray`, `TMap` |
| `b` | boolean variable | `bIsDead` |

Unreal style: PascalCase everywhere, tabs for indentation, braces on new lines.

## 7. Unreal core types

See the cheat sheet in the [part 4 README](../README.md#standard-c--unreal-cheat-sheet). Key points:
- `FString` for general text, `FName` for identifiers (fast compares, used for
  tags, sockets and asset names), and `FText` for anything shown to players
  (it supports localization).
- `TArray::Find`/`TMap::Find` return **pointers** (`nullptr` = not found),
  where the standard library uses iterators.
- Math: `FVector`, `FRotator` (Pitch, Yaw, Roll in degrees), `FQuat`,
  `FTransform`, `FMath::Clamp/Lerp/Sin/RandRange`.

## 8. Errors without exceptions

Unreal compiles with **exceptions disabled**. Instead:
| Tool | Behavior | Use for |
|------|----------|---------|
| `check(cond)` / `checkf(cond, fmt, ...)` | crash in dev builds if false; removed in Shipping | invariants that must never break |
| `verify(cond)` | like check, but the expression always runs | side-effecting calls |
| `ensure(cond)` / `ensureMsgf` | logs + debugger break **once**, continues; returns cond | "shouldn't happen, but survivable" |
| return values: `bool`, `TOptional`, `nullptr` | normal control flow | expected failures (like ch. 8's `optional`/`expected`) |
| `UE_LOG(LogCLearn, Error, TEXT(...))` | Output Log | diagnostics |

Idiomatic guard clause:
```cpp
if (!ensure(IsValid(Target))) { return; }
```

## 9. Logging

```cpp
// CLLog.h
CLEARNGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogCLearn, Log, All);
// CLLog.cpp
DEFINE_LOG_CATEGORY(LogCLearn);
// anywhere
UE_LOG(LogCLearn, Warning, TEXT("Orb %s collected by %s, value %d"), *GetName(), *Collector->GetName(), Value);
GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("On screen!"));
```
Verbosity levels: `Fatal, Error, Warning, Display, Log, Verbose, VeryVerbose`.
Window → Output Log shows them, and you can filter by category.

## 10. Blueprints vs C++

| C++ | Blueprints |
|-----|-----------|
| core systems, performance-critical code, complex logic | content wiring, tuning values, quick prototypes |
| base classes with `UPROPERTY`/`UFUNCTION` hooks | subclasses that pick meshes, sounds, materials |
| `BlueprintImplementableEvent` declares a hook | the designer implements it visually |

We write C++ base classes and create thin Blueprint subclasses (`BP_Orb`
derived from `ACLOrb`) to assign assets. That keeps asset paths out of C++.

## Walkthrough: `CLPrimerLibrary`

`CLearn/CLPrimerLibrary.h/.cpp` is a `UBlueprintFunctionLibrary`, a set of
static functions callable from any Blueprint. It shows `UENUM`, `USTRUCT`,
`FString::Printf/Format`, `FName`, `FText`, `TArray` with lambdas, `TMap::Find`,
and `check`/`ensure`.

Try it:
1. Build the project and open the editor.
2. Open the Level Blueprint (*Blueprints* toolbar button → *Open Level Blueprint*).
3. From *Event BeginPlay*, call **Run Primer Tour**.
4. Press Play and watch *Window → Output Log* (filter: `LogCLearn`).

Next: [exercises](exercises/README.md)
