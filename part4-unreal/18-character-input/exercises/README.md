# 18 — Exercises: Characters & Input

All three are solved in one subclass, `ACLAgileCharacter : ACLCharacter`, in
`solutions/CLAgileCharacter.h/.cpp`. Make `BP_CLCharacter` derive from it to try
it out (*Class Settings → Parent Class*).

### Ex 1 — Dash with cooldown ⭐⭐
Add `IA_Dash` (Digital, e.g. `Left Alt` / gamepad face button). Dashing launches
the character in its current movement direction, or facing direction when
standing still, with `LaunchCharacter`. Expose `DashStrength` and `DashCooldown`.
While on cooldown, the dash does nothing. Use a timer, not Tick.

### Ex 2 — Double jump & crouch toggle ⭐
Allow a double jump (hint: `JumpMaxCount`). Add `IA_Crouch` that **toggles**
crouching (`Crouch()` / `UnCrouch()`, and enable
`GetCharacterMovement()->NavAgentProps.bCanCrouch`). Crouching must not be
possible in the air.

### Ex 3 — Smooth camera zoom ⭐⭐
Add `IA_Zoom` (Axis1D, mouse wheel). Each wheel step changes a *target* arm
length, clamped to `[MinZoom, MaxZoom]`. Interpolate `CameraBoom->TargetArmLength`
toward it smoothly (`FMath::FInterpTo`) in Tick, and only tick while the length
is still changing.
