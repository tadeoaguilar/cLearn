#include "CLearn/CLPlayerState.h"

void ACLPlayerState::AddOrbs(int32 Amount)
{
	if (Amount == 0)
	{
		return;
	}
	Orbs = FMath::Max(0, Orbs + Amount);
	OnOrbsChanged.Broadcast(Orbs);
}

void ACLPlayerState::ResetOrbs()
{
	Orbs = 0;
	OnOrbsChanged.Broadcast(Orbs);
}
