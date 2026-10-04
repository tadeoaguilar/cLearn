// Chapter 19: per-player data that survives pawn death (the pawn is destroyed
// and respawned; the PlayerState isn't). Holds the orb count for the current run.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CLPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCLOnOrbsChanged, int32, NewOrbs);

UCLASS()
class CLEARNGAME_API ACLPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Orbs")
	void AddOrbs(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Orbs")
	void ResetOrbs();

	UFUNCTION(BlueprintPure, Category = "Orbs")
	int32 GetOrbs() const { return Orbs; }

	UFUNCTION(BlueprintPure, Category = "Orbs")
	int32 GetDeaths() const { return Deaths; }

	void AddDeath() { ++Deaths; }

	UPROPERTY(BlueprintAssignable, Category = "Orbs")
	FCLOnOrbsChanged OnOrbsChanged;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Orbs")
	int32 Orbs = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Orbs")
	int32 Deaths = 0;
};
