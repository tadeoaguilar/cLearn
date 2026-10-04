#include "CLearn/CLCharacter.h"

#include "Camera/CameraComponent.h"
#include "CLearn/CLGameMode.h"
#include "CLearn/CLHealthComponent.h"
#include "CLearn/CLInteractable.h"
#include "CLearn/CLLog.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "TimerManager.h"

ACLCharacter::ACLCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Rotate the character toward movement, not toward the camera (classic third-person feel)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 500.f, 0.f);
	Movement->JumpZVelocity = 700.f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = WalkSpeed;

	// Spring arm: keeps the camera behind the player and pulls it in when something blocks the view
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true; // the mouse rotates the arm

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Health = CreateDefaultSubobject<UCLHealthComponent>(TEXT("Health"));
}

void ACLCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // pick up Blueprint-edited values
	Health->OnDeath.AddDynamic(this, &ACLCharacter::HandleDeath);
	GetWorldTimerManager().SetTimer(FocusTimer, this, &ACLCharacter::UpdateFocus, 0.1f, true);
}

void ACLCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(FocusTimer);
	Super::EndPlay(EndPlayReason);
}

void ACLCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	// The mapping context (keys -> actions) lives on the LOCAL PLAYER's input subsystem.
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, /*Priority=*/0);
			}
		}
	}
}

void ACLCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!ensureMsgf(Input, TEXT("Enhanced Input is not the default input component class (Project Settings > Input)")))
	{
		return;
	}
	// ETriggerEvent: Started (pressed this frame), Triggered (every frame while active), Completed (released)
	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (MoveAction) Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACLCharacter::Move);
	if (LookAction) Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACLCharacter::Look);
	if (SprintAction)
	{
		Input->BindAction(SprintAction, ETriggerEvent::Started, this, &ACLCharacter::StartSprint);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACLCharacter::StopSprint);
	}
	if (InteractAction) Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ACLCharacter::Interact);
}

void ACLCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>(); // IA_Move is an Axis2D action: X = right, Y = forward
	if (!Controller)
	{
		return;
	}
	// Move relative to where the CAMERA looks, ignoring pitch so looking down doesn't slow you
	const FRotator YawOnly(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);
	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void ACLCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ACLCharacter::StartSprint() { GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; }

void ACLCharacter::StopSprint() { GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; }

AActor* ACLCharacter::TraceForInteractable() const
{
	const FVector Start = FollowCamera->GetComponentLocation();
	// Start from the camera but measure range from the character, so a long camera arm doesn't shorten reach
	const float CameraToCharacter = FVector::Dist(Start, GetActorLocation());
	const FVector End = Start + FollowCamera->GetForwardVector() * (CameraToCharacter + InteractRange);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CLInteractTrace), /*bTraceComplex=*/false, /*IgnoreActor=*/this);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UCLInteractable>()) // works for C++ AND Blueprint implementations
		{
			return HitActor;
		}
	}
	return nullptr;
}

void ACLCharacter::UpdateFocus()
{
	AActor* NewFocus = TraceForInteractable();
	if (NewFocus == FocusedActor.Get())
	{
		return;
	}
	FocusedActor = NewFocus;
	// Execute_X is how you call an interface function that might be implemented in Blueprint
	const FText Prompt = NewFocus ? ICLInteractable::Execute_GetInteractionPrompt(NewFocus) : FText::GetEmpty();
	OnInteractionPromptChanged.Broadcast(Prompt);
}

void ACLCharacter::Interact()
{
	if (AActor* Target = TraceForInteractable())
	{
		ICLInteractable::Execute_Interact(Target, this);
		FocusedActor = nullptr; // force a refresh: the prompt may have changed ("Open" -> "Close")
		UpdateFocus();
	}
}

void ACLCharacter::HandleDeath(UCLHealthComponent* /*HealthComponent*/, AController* /*Killer*/)
{
	UE_LOG(LogCLearn, Log, TEXT("%s died"), *GetName());
	AController* MyController = GetController();

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true); // ragdoll
	OnDied();

	if (ACLGameMode* GM = GetWorld()->GetAuthGameMode<ACLGameMode>())
	{
		GM->NotifyPlayerDied(MyController);
	}
	DetachFromControllerPendingDestroy(); // the controller can now possess a new pawn
	SetLifeSpan(5.f);                     // remove the body after a while
}
