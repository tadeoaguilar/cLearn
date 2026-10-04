#include "CLAgileCharacter.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "TimerManager.h"

ACLAgileCharacter::ACLAgileCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // only tick while zooming

	JumpMaxCount = 2;                                             // Ex 2: double jump — built into ACharacter
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;      // Ex 2: crouching must be enabled explicitly
}

void ACLAgileCharacter::BeginPlay()
{
	Super::BeginPlay();
	TargetArmLength = CameraBoom->TargetArmLength;
}

void ACLAgileCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent); // keep move/look/jump/sprint/interact
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (DashAction) Input->BindAction(DashAction, ETriggerEvent::Started, this, &ACLAgileCharacter::Dash);
		if (CrouchAction) Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &ACLAgileCharacter::ToggleCrouch);
		if (ZoomAction) Input->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ACLAgileCharacter::Zoom);
	}
}

// ---------------- Ex 1 ----------------
void ACLAgileCharacter::Dash()
{
	if (!bDashReady)
	{
		return;
	}
	FVector Direction = GetLastMovementInputVector(); // what the player is pressing right now
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}
	Direction.Z = 0.f;
	Direction.Normalize();

	// bXYOverride=true replaces horizontal velocity (snappy); bZOverride=false keeps gravity/jump
	LaunchCharacter(Direction * DashStrength, /*bXYOverride=*/true, /*bZOverride=*/false);

	bDashReady = false;
	GetWorldTimerManager().SetTimer(DashCooldownTimer, FTimerDelegate::CreateWeakLambda(this, [this] { bDashReady = true; }),
	                                FMath::Max(DashCooldown, 0.01f), false);
}

// ---------------- Ex 2 ----------------
void ACLAgileCharacter::ToggleCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else if (!GetCharacterMovement()->IsFalling()) // no crouching mid-air
	{
		Crouch();
	}
}

// ---------------- Ex 3 ----------------
void ACLAgileCharacter::Zoom(const FInputActionValue& Value)
{
	const float Steps = Value.Get<float>(); // wheel: +1 / -1
	TargetArmLength = FMath::Clamp(TargetArmLength - Steps * ZoomStep, MinZoom, MaxZoom);
	SetActorTickEnabled(true);
}

void ACLAgileCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaSeconds, ZoomSpeed);
	if (FMath::IsNearlyEqual(CameraBoom->TargetArmLength, TargetArmLength, 0.5f))
	{
		CameraBoom->TargetArmLength = TargetArmLength;
		SetActorTickEnabled(false);
	}
}
