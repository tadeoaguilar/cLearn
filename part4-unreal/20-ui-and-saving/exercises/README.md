# 20 — Exercises: UI & Saving

### Ex 1 — Pause menu ⭐⭐
Create `UCLPauseMenuWidget` with three `BindWidget` buttons: `ResumeButton`,
`RestartButton` and `QuitButton` (bind `OnClicked` with `AddDynamic` in
`NativeConstruct`). Add a `ACLPauseController : ACLPlayerController` that binds
an `IA_Pause` action (Escape / gamepad Start). Pausing must:
`SetPause(true)`, show the menu, `SetInputMode(FInputModeUIOnly)` with the mouse
cursor visible, and focus the Resume button. Resuming undoes all of it.
Hint: the action must keep working while paused. Enable *Trigger when Paused* on
the `IA_Pause` asset (`UInputAction::bTriggerWhenPaused`).
→ `solutions/CLPauseMenuWidget.h/.cpp` + `solutions/CLPauseController.h/.cpp`

### Ex 2 — Remember collected orbs ⭐⭐⭐
Make collected orbs stay collected across sessions, like a Metroidvania:
- `UCLWorldStateSubsystem` (GameInstanceSubsystem) owns its own `USaveGame`
  (`UCLWorldSaveGame`) with `TMap<FName, FCLCollectedSet>` (level name → set of
  actor ids).
- `ACLPersistentOrb : ACLOrb` destroys itself in `BeginPlay` if already
  collected, and records itself when destroyed during play (hint: `EndPlay`
  with reason `Destroyed`).
- Add `ResetLevel()` to forget the current level's orbs.
Why can't you just override `OnCollected` in C++?
→ `solutions/CLWorldStateSubsystem.h/.cpp` + `solutions/CLPersistentOrb.h/.cpp`

### Ex 3 — Settings: mouse sensitivity ⭐⭐
`UCLSettingsWidget` has a `USlider` called `SensitivitySlider` (0.1–3) and a
`UTextBlock` called `SensitivityLabel`. Load the value from
`UCLSaveSubsystem::GetSave()->MouseSensitivity` on construct, and update the save
on change (debounced: save 0.5 s after the last change, not on every pixel of
slider movement). Then apply it in `ACLCharacter::Look`.
→ `solutions/CLSettingsWidget.h/.cpp` (the `Look` change is shown in the header comment)
