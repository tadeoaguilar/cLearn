# 18 — Characters, Controllers & Enhanced Input

> Goal: a controllable third-person character, written from scratch in C++. It
> covers the Pawn/Controller split, the character movement component, the
> camera rig, and the **Enhanced Input** system.

## 1. Who's who: the gameplay framework

```
 GameMode  (rules, server only) ── spawns ──►  PlayerController  (the player's will: input, HUD, camera)
                                                    │ possesses
                                                    ▼
 PlayerState (score, name — survives death)     Pawn / Character  (the body in the world)
```
| Class | Lifetime | Holds |
|-------|----------|-------|
| `AGameModeBase` | per level, server only | rules: spawning, win/lose, respawn (ch. 19) |
| `APlayerController` | per player, whole session in the level | input routing, HUD (ch. 20), camera manager |
| `APlayerState` | per player, survives pawn death | score, deaths (ch. 19) |
| `APawn` / `ACharacter` | dies and respawns | mesh, movement, health |

Separating **controller** (will) from **pawn** (body) is why respawning is easy:
the controller simply possesses a new pawn. An AI controller can drive the very
same character class.

`ACharacter` = `APawn` + capsule collision + skeletal mesh +
`UCharacterMovementComponent`, which gives you walking, falling, jumping,
crouching, swimming, slopes, steps and network prediction for free.

## 2. The camera rig

```cpp
CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
CameraBoom->SetupAttachment(RootComponent);
CameraBoom->TargetArmLength = 400.f;
CameraBoom->bUsePawnControlRotation = true;          // mouse rotates the arm (orbit)
FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // end of the arm
```
The **spring arm** pulls the camera in when a wall gets between it and the
player. Combined with `bOrientRotationToMovement = true` and
`bUseControllerRotationYaw = false`, the camera orbits freely, and the
character turns to face the way it runs.

## 3. Enhanced Input

Enhanced Input is UE5's input system. It separates **what the player wants to
do** from **which keys do it**:

```
 Keys/sticks ──► Input Mapping Context (IMC_Default) ──► Input Actions (IA_Move, IA_Jump...)
                 W/A/S/D + left stick → IA_Move          value type: bool / Axis1D / Axis2D / Axis3D
                 modifiers (negate, swizzle, deadzone)    triggers (pressed, held, tap, chorded)
                                                              │
                                                  C++: BindAction(IA_Move, Triggered, this, &Move)
```
- **Input Action** (`UInputAction` asset): a semantic action with a value type.
- **Input Mapping Context** (`UInputMappingContext` asset): binds keys to
  actions, with **modifiers** (e.g. `S` = negated Y) and **triggers**.
  Contexts can be added and removed at runtime: on foot, driving, menus.
- Our character takes them as `UPROPERTY(EditDefaultsOnly)` asset references,
  so **no asset paths are hard-coded in C++**.

### Binding (from `CLCharacter.cpp`)
```cpp
void ACLCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    Input->BindAction(JumpAction,  ETriggerEvent::Started,   this, &ACharacter::Jump);
    Input->BindAction(JumpAction,  ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    Input->BindAction(MoveAction,  ETriggerEvent::Triggered, this, &ACLCharacter::Move);
    ...
}
void ACLCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    const FRotator YawOnly(0, Controller->GetControlRotation().Yaw, 0);
    AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X), Axis.Y);   // forward
    AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y), Axis.X);   // right
}
```
| `ETriggerEvent` | Fires |
|-----------------|-------|
| `Started` | the frame the action begins (key down) |
| `Triggered` | every frame the trigger condition holds (held stick, held key) |
| `Completed` | when it ends (key up) |
| `Ongoing`, `Canceled` | for multi-stage triggers (hold, tap) |

### Adding the mapping context
Contexts live on the **local player's** `UEnhancedInputLocalPlayerSubsystem`. We
add ours in `PawnClientRestart()`, which runs whenever this pawn is possessed
locally, including after a respawn:
```cpp
if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    Subsystem->AddMappingContext(DefaultMappingContext, 0);
```

## 4. Movement relative to the camera

Pressing *forward* should move toward where the **camera** looks, not where the
character faces. We take only the controller's **yaw**. Pitch is ignored, so
looking down doesn't slow you down. Then we derive forward and right vectors
with `FRotationMatrix`. Sprinting just swaps
`GetCharacterMovement()->MaxWalkSpeed` between `WalkSpeed` and `SprintSpeed`.

## 5. Line traces: "what am I looking at?"

```cpp
FHitResult Hit;
FCollisionQueryParams Params(SCENE_QUERY_STAT(CLInteractTrace), false, this); // ignore ourselves
if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    if (Hit.GetActor()->Implements<UCLInteractable>()) ...
```
`ACLCharacter` traces from the camera 10 times per second (a timer, not Tick),
and broadcasts `OnInteractionPromptChanged` only when the target **changes**.
The HUD displays it (ch. 20), and the Interact action calls the interface (ch. 19).
Debug traces visually with `DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f)`
(`#include "DrawDebugHelpers.h"`).

## In the editor

1. **Input assets**: the Third Person template already has `IMC_Default`,
   `IA_Move`, `IA_Look` and `IA_Jump` (in `Content/ThirdPerson/Input` or
   `Content/Input`). Create two more Input Actions: **`IA_Sprint`** (Digital/bool)
   and **`IA_Interact`** (Digital). Open `IMC_Default` and map
   `Left Shift → IA_Sprint`, `E → IA_Interact` (plus gamepad buttons if you like).
2. **BP_CLCharacter**: create a Blueprint with parent `CLCharacter`. In the
   *Mesh* component, set the template's skeletal mesh (`SKM_Manny`/`SKM_Quinn`)
   and Anim Class (`ABP_Manny`/`ABP_Quinn`), at location Z = -96 and rotation
   Yaw = -90. In *Class Defaults → Input*, assign the mapping context and the 5 actions.
3. **BP_CLPlayerController**: a Blueprint with parent `CLPlayerController`.
   You'll set its HUD class in ch. 20.
4. **BP_CLGameMode**: parent `CLGameMode`. Set *Default Pawn Class =
   BP_CLCharacter* and *Player Controller Class = BP_CLPlayerController*.
5. *World Settings → GameMode Override = BP_CLGameMode* (or make it the project
   default in *Project Settings → Maps & Modes*), then press Play.

Next: [exercises](exercises/README.md)
