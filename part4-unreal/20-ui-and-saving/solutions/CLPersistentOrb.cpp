#include "CLPersistentOrb.h"

#include "CLWorldStateSubsystem.h"
#include "Engine/GameInstance.h"

void ACLPersistentOrb::BeginPlay()
{
	Super::BeginPlay();
	if (const UCLWorldStateSubsystem* State = GetGameInstance()->GetSubsystem<UCLWorldStateSubsystem>())
	{
		if (State->IsCollected(this))
		{
			bRestoredAsCollected = true;
			Destroy();
		}
	}
	// Note: ACLGameMode counts orbs in ITS BeginPlay. In real code, have the GameMode skip
	// already-collected orbs (or count orbs in StartPlay after actors initialized).
}

void ACLPersistentOrb::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Destroyed during play = collected (level unload / PIE stop use other reasons)
	if (EndPlayReason == EEndPlayReason::Destroyed && !bRestoredAsCollected)
	{
		if (UCLWorldStateSubsystem* State = GetGameInstance()->GetSubsystem<UCLWorldStateSubsystem>())
		{
			State->MarkCollected(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}
