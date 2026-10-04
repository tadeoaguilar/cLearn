#include "CLCheckpointGameMode.h"

#include "CLCheckpoint.h"

void ACLCheckpointGameMode::SetCheckpoint(AController* Player, ACLCheckpoint* Checkpoint)
{
	if (!Player || !Checkpoint)
	{
		return;
	}
	if (TWeakObjectPtr<ACLCheckpoint>* Previous = Checkpoints.Find(Player))
	{
		if (ACLCheckpoint* Old = Previous->Get(); Old && Old != Checkpoint)
		{
			Old->Deactivate(); // only the newest checkpoint is active
		}
	}
	Checkpoints.Add(Player, Checkpoint);
}

void ACLCheckpointGameMode::RestartPlayer(AController* NewPlayer)
{
	if (const TWeakObjectPtr<ACLCheckpoint>* Found = Checkpoints.Find(NewPlayer))
	{
		if (const ACLCheckpoint* Checkpoint = Found->Get())
		{
			RestartPlayerAtTransform(NewPlayer, Checkpoint->GetSpawnTransform());
			return;
		}
	}
	Super::RestartPlayer(NewPlayer); // no checkpoint yet: use a PlayerStart
}
