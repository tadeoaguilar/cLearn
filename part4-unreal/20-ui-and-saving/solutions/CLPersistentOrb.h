#pragma once

#include "CoreMinimal.h"
#include "CLearn/CLOrb.h"
#include "CLPersistentOrb.generated.h"

// An orb that stays collected across sessions.
//
// Why not override OnCollected? It's a BlueprintImplementableEvent: it has no C++
// body and isn't virtual, so C++ subclasses can't override it. (A BlueprintNativeEvent
// would allow it via OnCollected_Implementation.) Instead we hook EndPlay, which
// tells us WHY the actor is leaving play.
UCLASS()
class CLEARNGAME_API ACLPersistentOrb : public ACLOrb
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool bRestoredAsCollected = false;
};
