# Part 4 — Game Development with Unreal Engine 5 (C++)

You now know modern C++: classes, RAII, templates, lambdas, containers and
concurrency. Unreal Engine uses all of these, but with **its own dialect**:
reflection macros, garbage-collected `UObject`s, its own containers and strings,
and no exceptions. Part 4 teaches that dialect by building a small third-person
game, **"Orb Runner"**:

> Run around the level collecting orbs, avoid damage zones, open doors by
> interacting with them, and beat your best time. Your progress is saved to
> disk. As a bonus, the in-game **quest log is loaded from the Tasks API you
> built in part 3**, and finishing a quest marks the task `done` over HTTP.

| # | Chapter | You will build |
|---|---------|----------------|
| 16 | [Unreal C++ Primer](16-unreal-primer/README.md) | project setup, reflection, UObject/GC, UE types, logging |
| 17 | [Actors & Components](17-actors-components/README.md) | rotating orb actor, health component, spawner, lifecycle logger |
| 18 | [Characters & Enhanced Input](18-character-input/README.md) | `ACLCharacter` with camera, move/look/jump/sprint/interact |
| 19 | [Gameplay Systems](19-gameplay-systems/README.md) | damage, delegates, interfaces (doors), PlayerState score, GameMode rules & respawn |
| 20 | [UI & Save Games](20-ui-and-saving/README.md) | UMG HUD from C++, `USaveGame` persistence, HTTP quest log from the Tasks API |

## How the code is organized

```
part4-unreal/
├── CLearnGame/                      ← drop-in source for YOUR Unreal project
│   ├── Source/CLearnGame/
│   │   ├── CLearnGame.Build.cs      ← module dependencies (replace the template's)
│   │   └── CLearn/                  ← all our gameplay classes (prefix "CL")
│   └── Config/DefaultGame.ini       ← settings snippet (quest API URL)
├── 16-unreal-primer/ ... 20-ui-and-saving/
│   ├── README.md                    ← concepts + walkthrough of that chapter's classes
│   ├── exercises/README.md
│   └── solutions/                   ← extra classes that solve the exercises (drop into CLearn/)
```

## Setup (do this once)

1. Install **Unreal Engine 5.4 or newer** from the Epic Games Launcher, plus an IDE:
   - macOS: full **Xcode** (App Store), then open Xcode once to accept the license.
   - Windows: **Visual Studio 2022** with "Game development with C++", or **JetBrains Rider**.
2. In the launcher, create a project: **Games → Third Person → C++**, named
   **`CLearnGame`** (the name matters: our code uses the `CLEARNGAME_API` macro).
   Starter content is optional.
3. Close the editor. Copy `part4-unreal/CLearnGame/Source/CLearnGame/CLearn/` into
   `<YourProject>/Source/CLearnGame/`, and replace
   `<YourProject>/Source/CLearnGame/CLearnGame.Build.cs` with ours.
4. Append `part4-unreal/CLearnGame/Config/DefaultGame.ini` to your project's
   `Config/DefaultGame.ini`.
5. Regenerate project files: right-click `CLearnGame.uproject` → *Generate
   Visual Studio project files*. On macOS, run
   `"/Users/Shared/Epic Games/UE_5.x/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh" -project="$PWD/CLearnGame.uproject" -game`.
6. Build and open: from the IDE (target `CLearnGameEditor`, Development Editor),
   or double-click the `.uproject` and accept the rebuild prompt.

Each chapter then tells you which Blueprints to create in the editor. Our C++
classes expose properties and events, and Blueprints supply the meshes,
materials, sounds and level layout. That's the standard division of labor in
Unreal teams.

> **Note on verification.** Parts 1–3 of this repository were compiled and
> tested automatically. Unreal code can only be built inside an Unreal project
> with the engine installed, so part 4 was written carefully against the
> UE 5.4/5.5 APIs but not compiled here. If your engine version reports an
> error, the chapter READMEs explain the API involved, so you can adapt it.
> Fixing it is itself good practice.

## Standard C++ → Unreal cheat sheet

| Standard C++ | Unreal | Notes |
|--------------|--------|-------|
| `int`, `std::int32_t`, `float` | `int32`, `uint8`, `float`, `double` | fixed-size aliases everywhere |
| `std::string` | `FString` | mutable, `TCHAR` (UTF-16 on Windows) |
| — | `FName` | interned, case-insensitive identifier: cheap to compare |
| — | `FText` | localizable, user-facing text |
| `"literal"` | `TEXT("literal")` | wide-character literal |
| `std::vector<T>` | `TArray<T>` | `Add`, `Emplace`, `Remove`, `Num()`, range-for |
| `std::unordered_map<K,V>` | `TMap<K,V>` | `Add`, `Find` (returns a pointer), `Contains` |
| `std::unordered_set<T>` | `TSet<T>` | |
| `std::unique_ptr<T>` | `TUniquePtr<T>` | non-UObjects only |
| `std::shared_ptr` / `weak_ptr` | `TSharedPtr` / `TSharedRef` / `TWeakPtr` | non-UObjects only |
| raw pointer to a UObject | `TObjectPtr<T>` in a `UPROPERTY()` | the GC tracks it |
| `std::weak_ptr` to a UObject | `TWeakObjectPtr<T>` | doesn't keep it alive |
| `std::function` | `TFunction`, delegates (`DECLARE_DELEGATE...`) | delegates can bind to UObjects safely |
| `std::optional<T>` | `TOptional<T>` | |
| exceptions | **none**: `check()`, `ensure()`, return values, logs | UE builds with exceptions disabled |
| `std::cout` | `UE_LOG(LogCategory, Warning, TEXT("x=%d"), X)` | |
| `std::thread` | `Async`, `UE::Tasks`, `FRunnable` | game logic runs on the **game thread** |
| `std::chrono` | `FDateTime`, `FTimespan`, `GetWorld()->GetTimeSeconds()` | plus timers via `FTimerManager` |

The standard library is still available, but engine APIs expect Unreal types, so
use them in gameplay code.
