// Chapter 19: the rules of the game. Exists only on the server (in single-player:
// the local game). Counts orbs, measures run time, respawns dead players, and
// decides when a run is won or lost.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CLGameMode.generated.h"

class ACLPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCLOnRunEnded, bool, bWon, float, ElapsedSeconds, bool, bNewRecord);

UCLASS()
class CLEARNGAME_API ACLGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACLGameMode();

	void NotifyOrbCollected(ACLPlayerState* Collector);
	void NotifyPlayerDied(AController* Victim);

	UFUNCTION(BlueprintPure, Category = "Run")
	float GetElapsedSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Run")
	float GetRemainingSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Run")
	int32 GetTotalOrbs() const { return TotalOrbs; }

	UFUNCTION(BlueprintPure, Category = "Run")
	bool IsRunOver() const { return bRunOver; }

	UFUNCTION(BlueprintCallable, Category = "Run")
	void RestartRun();

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FCLOnRunEnded OnRunEnded;

protected:
	virtual void BeginPlay() override;

	// 0 = no time limit
	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "0", Units = "s"))
	float TimeLimitSeconds = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "0", Units = "s"))
	float RespawnDelay = 3.f;

	// Each death costs this many orbs (0 = forgiving)
	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "0"))
	int32 OrbPenaltyOnDeath = 2;

private:
	void EndRun(bool bWon);
	void HandleTimeUp();
	void Respawn(TWeakObjectPtr<AController> Controller); // weak: the controller may be gone by then

	int32 TotalOrbs = 0;
	double RunStartTime = 0.0;
	double RunEndTime = 0.0;
	bool bRunOver = false;
	FTimerHandle TimeLimitTimer;
};
