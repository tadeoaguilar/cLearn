# 20 — UI (UMG), Save Games & Talking to Your API

> Goal: show game state on screen with UMG widgets driven from C++, persist
> progress with `USaveGame`, and close the loop with part 3 by loading the
> **quest log from your Tasks API over HTTP**.

## 1. UMG: C++ logic, Blueprint layout

UMG (Unreal Motion Graphics) is Unreal's UI framework. The best practice is:
- a **C++ base class** (`UCLHUDWidget : UUserWidget`) holds the logic: which
  data, which events, formatting,
- a **Widget Blueprint** (`WBP_HUD`, parent class `CLHUDWidget`) holds the
  layout: anchors, fonts, colors and animations, which designers iterate on
  without recompiling.

The bridge is `meta = (BindWidget)`:
```cpp
UPROPERTY(meta = (BindWidget))         TObjectPtr<UProgressBar> HealthBar;   // REQUIRED in WBP_HUD, same name & type
UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock>  PromptText;  // may be missing → check for null
```
If `WBP_HUD` lacks a widget called `HealthBar` of type Progress Bar, the
Blueprint **fails to compile** with a clear message. That's a contract between
programmer and designer.

### Widget lifecycle
| C++ | When |
|-----|------|
| `CreateWidget<T>(PlayerController, Class)` | create (in `ACLPlayerController::BeginPlay`) |
| `AddToViewport()` / `RemoveFromParent()` | show / hide |
| `NativeConstruct()` | after the widget tree is built: bind delegates here |
| `NativeTick()` | every frame while visible: use sparingly (we only update the clock) |
| `NativeDestruct()` | unbind from long-lived objects (subsystems!) |

### Event-driven UI
The HUD **subscribes** to delegates instead of polling every frame:
- `HealthComponent->OnHealthChanged` → bar percent (+ `OnDamaged` BP event for a flash animation)
- `PlayerState->OnOrbsChanged` → "Orbs 3 / 10"
- `Character->OnInteractionPromptChanged` → "[E] Open door"
- `GameMode->OnRunEnded` → "You win! 01:23.45 NEW RECORD!"
- `QuestSubsystem->OnQuestsUpdated` → rebuilds the quest list

**Respawn subtlety**: the pawn (and its health component) is replaced on
respawn. `ACLPlayerController::OnPossess` calls `HUD->BindToPawn(NewPawn)`,
which **unbinds** from the old pawn (`RemoveAll(this)`) and binds to the new one.
The PlayerState persists, so `AddUniqueDynamic` prevents binding it twice.

Runtime-created widgets come from the widget's own `WidgetTree`:
```cpp
UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
QuestList->AddChildToVerticalBox(Line);
```

### Text formatting for players
Use `FText` (localizable) for anything on screen:
```cpp
#define LOCTEXT_NAMESPACE "CLHUD"
OrbsText->SetText(FText::Format(LOCTEXT("Orbs", "Orbs {0} / {1}"), NewOrbs, Total));
#undef LOCTEXT_NAMESPACE
```

## 2. Save games

```
UCLSaveGame : USaveGame            the DATA (UPROPERTYs are serialized)
UCLSaveSubsystem : UGameInstanceSubsystem   the SERVICE (load at startup, save on changes)
```
```cpp
UGameplayStatics::DoesSaveGameExist(Slot, UserIndex);
USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(Slot, UserIndex);   // Cast<UCLSaveGame>!
UGameplayStatics::SaveGameToSlot(Save, Slot, UserIndex);                   // blocking
UGameplayStatics::AsyncSaveGameToSlot(Save, Slot, UserIndex, Delegate);    // disk write on a worker thread
```
Files go to `Saved/SaveGames/<Slot>.sav` (on desktop platforms).

Design decisions (the same ones as in ch. 13):
- **Where does the save live?** In a **GameInstanceSubsystem**. The engine
  creates it with the GameInstance, and it survives level loads. Our level
  restart (`OpenLevel`) destroys every actor, but not the subsystem.
- **When to save?** After meaningful events (a run ends), with an **async** save
  so there's no frame hitch, plus a **sync** save in `Deinitialize` on quit.
- **Versioning**: `SaveVersion` + `Migrate()` upgrades old files, like the SQL
  migrations of ch. 14. Unreal's serializer tolerates added fields (they get
  default values). For renamed fields, use Core Redirects.
- **Corruption**: `LoadGameFromSlot` returns null or the wrong class → start fresh,
  and log it.
- Saving actor state (positions, collected items) is usually done by giving
  actors a stable id (`FName`/`FGuid`) and storing `id → state` maps. See exercise 2.

## 3. HTTP: the quest log comes from part 3

`UCLQuestSubsystem` (also a GameInstanceSubsystem) calls the **Tasks API** you
built in C++ in part 3:

```
 Game start ──GET /tasks?status=todo&sort=priority──► Tasks API ──► PostgreSQL
                       ◄──── {"items":[{"id":7,"title":"Collect every orb","description":"event:all_orbs",...}]}
 Run won ── ReportEvent("all_orbs") ──PATCH /tasks/7 {"status":"done"}──►
```
```cpp
TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
Request->SetURL(BaseUrl + TEXT("/tasks?status=todo"));
Request->SetVerb(TEXT("GET"));
Request->OnProcessRequestComplete().BindUObject(this, &UCLQuestSubsystem::HandleQuestsResponse);
Request->ProcessRequest();   // async; the callback runs on the game thread later
```
- **Never block the game thread on the network.** HTTP calls are asynchronous,
  and the callback arrives on the game thread, so it's safe to touch UObjects there.
- **Offline is normal for games.** If the server isn't running, we log a
  warning and the HUD shows "Quest log offline". The game is fully playable.
- JSON with the engine's `Json` module: `TJsonReaderFactory` +
  `FJsonSerializer::Deserialize` → `FJsonObject` → `TryGetArrayField`,
  `TryGetStringField`. Writing uses `TJsonWriterFactory` + `FJsonSerializer::Serialize`.
- **Data-driven link**: a task whose description contains `event:<id>` is
  completed when the game calls `ReportEvent(<id>)`. `ACLGameMode` reports
  `all_orbs` on a win, and `ACLDoor` reports `door_opened`.
- **Configuration** comes from `DefaultGame.ini` through `UPROPERTY(Config)`
  (`BaseUrl`, optional `ApiKey`), so there's no recompile to point at another server.

### Try the full loop
```bash
cd part3-crud-api && docker compose up -d --build      # API on :8080
curl -X POST localhost:8080/tasks -H 'Content-Type: application/json' \
     -d '{"title":"Collect every orb","description":"event:all_orbs","priority":"high"}'
curl -X POST localhost:8080/tasks -H 'Content-Type: application/json' \
     -d '{"title":"Find the hidden door","description":"event:door_opened"}'
```
Press Play in Unreal. The quests appear in the HUD. Open the door: "Quest
complete: Find the hidden door". Then check it on the server:
```bash
curl 'localhost:8080/tasks?status=done'
```
macOS and iOS block plain HTTP to non-local hosts by default (App Transport
Security), and so do some consoles and mobile platforms. `localhost` is fine
for development. Use HTTPS in production.

## Walkthrough of this chapter's classes

| File | Concepts |
|------|----------|
| `CLHUDWidget.h/.cpp` | `BindWidget`, delegates → UI, rebinding on respawn, runtime child widgets, `FText::Format`, lambda timers with weak `this` |
| `CLPlayerController.h/.cpp` | creating the HUD, `OnPossess` rebinding, input mode |
| `CLSaveGame.h` | `USaveGame`, versioned data, nested `USTRUCT` arrays, `FDateTime` |
| `CLSaveSubsystem.h/.cpp` | `UGameInstanceSubsystem`, load/migrate/async save, sync save on shutdown |
| `CLQuestSubsystem.h/.cpp` | `UCLASS(Config=Game)`, HTTP GET/PATCH, JSON parse/write, delegate payloads, offline handling |

## In the editor

1. **WBP_HUD**: *Widget Blueprint*. In *Graph → Class Settings*, set
   *Parent Class = CLHUDWidget*. In the Designer, add:
   - a **Progress Bar** named `HealthBar` (top-left),
   - **Text** named `OrbsText` and `TimerText` (top-right),
   - optional: **Text** `PromptText` (center-bottom), `MessageText` (center),
     `BestTimeText`, and a **Vertical Box** `QuestList` (left side).
   Compile. If a required name is missing, you'll see the BindWidget error.
2. In **BP_CLPlayerController**, set *HUD Widget Class = WBP_HUD*.
3. Optional polish: in WBP_HUD's graph, implement **On Damaged** (play a red
   flash animation) and **On Run Finished**.
4. Play, finish a run, quit, and play again. The best time persists. Look in
   `Saved/SaveGames/CLearnSave.sav`.

Next: [exercises](exercises/README.md)
