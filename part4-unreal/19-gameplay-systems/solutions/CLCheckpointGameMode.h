#pragma once

#include "CoreMinimal.h"
#include "CLearn/CLGameMode.h"
#include "CLCheckpointGameMode.generated.h"

class ACLCheckpoint;

// Extends the chapter's GameMode: respawn at the last checkpoint each player touched.
UCLASS()
class CLEARNGAME_API ACLCheckpointGameMode : public ACLGameMode
{
	GENERATED_BODY()

public:
	void SetCheckpoint(AController* Player, ACLCheckpoint* Checkpoint);

	// RestartPlayer -> FindPlayerStart -> ChoosePlayerStart; RestartPlayerAtTransform lets us bypass PlayerStarts.
	virtual void RestartPlayer(AController* NewPlayer) override;

private:
	// Weak keys/values: controllers and checkpoints may be destroyed (level unload, logout)
	TMap<TWeakObjectPtr<AController>, TWeakObjectPtr<ACLCheckpoint>> Checkpoints;
};
