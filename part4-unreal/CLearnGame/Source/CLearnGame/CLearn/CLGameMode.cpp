#include "CLearn/CLGameMode.h"

#include "CLearn/CLCharacter.h"
#include "CLearn/CLLog.h"
#include "CLearn/CLOrb.h"
#include "CLearn/CLPlayerController.h"
#include "CLearn/CLPlayerState.h"
#include "CLearn/CLQuestSubsystem.h"
#include "CLearn/CLSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h" // TActorIterator
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACLGameMode::ACLGameMode()
{
	// C++ defaults; a Blueprint subclass (BP_CLGameMode) swaps in BP_CLCharacter / BP_CLPlayerController.
	DefaultPawnClass = ACLCharacter::StaticClass();
	PlayerControllerClass = ACLPlayerController::StaticClass();
	PlayerStateClass = ACLPlayerState::StaticClass();
}

void ACLGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Iterate over every orb placed in the level
	TotalOrbs = 0;
	for (TActorIterator<ACLOrb> It(GetWorld()); It; ++It)
	{
		TotalOrbs += It->GetValue();
	}
	RunStartTime = GetWorld()->GetTimeSeconds();
	if (TimeLimitSeconds > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimeLimitTimer, this, &ACLGameMode::HandleTimeUp, TimeLimitSeconds, false);
	}
	UE_LOG(LogCLearn, Log, TEXT("Run started: %d orbs to collect, time limit %.0fs"), TotalOrbs, TimeLimitSeconds);
}

float ACLGameMode::GetElapsedSeconds() const
{
	const double End = bRunOver ? RunEndTime : GetWorld()->GetTimeSeconds();
	return static_cast<float>(End - RunStartTime);
}

float ACLGameMode::GetRemainingSeconds() const
{
	return TimeLimitSeconds > 0.f ? FMath::Max(0.f, TimeLimitSeconds - GetElapsedSeconds()) : 0.f;
}

void ACLGameMode::NotifyOrbCollected(ACLPlayerState* Collector)
{
	if (!bRunOver && Collector && TotalOrbs > 0 && Collector->GetOrbs() >= TotalOrbs)
	{
		EndRun(/*bWon=*/true);
	}
}

void ACLGameMode::NotifyPlayerDied(AController* Victim)
{
	if (!Victim)
	{
		return;
	}
	if (ACLPlayerState* PS = Victim->GetPlayerState<ACLPlayerState>())
	{
		PS->AddDeath();
		PS->AddOrbs(-OrbPenaltyOnDeath);
	}
	if (bRunOver)
	{
		return;
	}
	// FTimerDelegate::CreateUObject binds a member function WITH an extra argument (payload).
	// The weak binding means the timer is skipped safely if the GameMode is destroyed first.
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle,
	                                FTimerDelegate::CreateUObject(this, &ACLGameMode::Respawn, TWeakObjectPtr<AController>(Victim)),
	                                RespawnDelay, false);
}

void ACLGameMode::Respawn(TWeakObjectPtr<AController> Controller)
{
	AController* C = Controller.Get(); // nullptr if it was destroyed meanwhile
	if (C && !C->GetPawn())
	{
		RestartPlayer(C); // spawns DefaultPawnClass at a PlayerStart and possesses it
	}
}

void ACLGameMode::HandleTimeUp()
{
	if (!bRunOver)
	{
		EndRun(/*bWon=*/false);
	}
}

void ACLGameMode::EndRun(bool bWon)
{
	bRunOver = true;
	RunEndTime = GetWorld()->GetTimeSeconds();
	GetWorldTimerManager().ClearTimer(TimeLimitTimer);
	const float Elapsed = GetElapsedSeconds();

	int32 Orbs = 0;
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (const ACLPlayerState* PS = PC->GetPlayerState<ACLPlayerState>())
		{
			Orbs = PS->GetOrbs();
		}
	}

	bool bNewRecord = false;
	UGameInstance* GI = GetGameInstance();
	if (UCLSaveSubsystem* Saves = GI->GetSubsystem<UCLSaveSubsystem>())
	{
		bNewRecord = Saves->RecordRun(bWon, Elapsed, Orbs); // persisted to disk (ch. 20)
	}
	if (bWon)
	{
		if (UCLQuestSubsystem* Quests = GI->GetSubsystem<UCLQuestSubsystem>())
		{
			Quests->ReportEvent(TEXT("all_orbs")); // marks matching tasks done in the Tasks API (ch. 20)
		}
	}

	UE_LOG(LogCLearn, Log, TEXT("Run %s in %.2fs with %d orbs%s"), bWon ? TEXT("WON") : TEXT("LOST"), Elapsed, Orbs,
	       bNewRecord ? TEXT(" — NEW RECORD!") : TEXT(""));
	OnRunEnded.Broadcast(bWon, Elapsed, bNewRecord);
}

void ACLGameMode::RestartRun()
{
	// Reloading the level resets every actor; the GameInstance (and its subsystems) survive.
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
