#include "CLearn/CLDoor.h"

#include "CLearn/CLLog.h"
#include "CLearn/CLPlayerState.h"
#include "CLearn/CLQuestSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"

#define LOCTEXT_NAMESPACE "CLDoor"

ACLDoor::ACLDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // only tick while animating

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	Hinge->SetupAttachment(Root);
	Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel"));
	Panel->SetupAttachment(Hinge);
	Panel->SetCollisionProfileName(TEXT("BlockAll")); // also blocks the Visibility trace -> interactable
}

void ACLDoor::Interact_Implementation(AActor* Interactor)
{
	if (!bOpen && RequiredOrbs > 0)
	{
		const APawn* Pawn = Cast<APawn>(Interactor);
		const ACLPlayerState* PS = Pawn ? Pawn->GetPlayerState<ACLPlayerState>() : nullptr;
		if (!PS || PS->GetOrbs() < RequiredOrbs)
		{
			UE_LOG(LogCLearn, Log, TEXT("%s is locked: needs %d orbs"), *GetName(), RequiredOrbs);
			OnLocked(Interactor);
			return;
		}
	}

	bOpen = !bOpen;
	SetActorTickEnabled(true); // animate in Tick until we reach the target angle
	OnDoorStateChanged(bOpen);

	if (bOpen && !bReportedQuest && !QuestEventId.IsNone())
	{
		bReportedQuest = true;
		if (UCLQuestSubsystem* Quests = GetGameInstance()->GetSubsystem<UCLQuestSubsystem>())
		{
			Quests->ReportEvent(QuestEventId);
		}
	}
}

FText ACLDoor::GetInteractionPrompt_Implementation() const
{
	if (!bOpen && RequiredOrbs > 0)
	{
		return FText::Format(LOCTEXT("Locked", "Locked ({0} orbs needed)"), RequiredOrbs);
	}
	return bOpen ? LOCTEXT("Close", "Close door") : LOCTEXT("Open", "Open door");
}

void ACLDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Target = bOpen ? OpenAngle : 0.f;
	CurrentYaw = FMath::FInterpTo(CurrentYaw, Target, DeltaSeconds, OpenSpeed); // smooth ease-out
	Hinge->SetRelativeRotation(FRotator(0.f, CurrentYaw, 0.f));
	if (FMath::IsNearlyEqual(CurrentYaw, Target, 0.1f))
	{
		Hinge->SetRelativeRotation(FRotator(0.f, Target, 0.f));
		SetActorTickEnabled(false); // done: stop paying for Tick
	}
}

#undef LOCTEXT_NAMESPACE
