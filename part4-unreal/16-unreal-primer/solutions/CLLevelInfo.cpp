#include "CLLevelInfo.h"

bool UCLLevelInfoLibrary::ValidateLevelInfo(const FCLLevelInfo& Info, FString& OutError)
{
	if (Info.LevelId.IsNone())
	{
		OutError = TEXT("LevelId must be set");
		return false;
	}
	if (Info.OrbCount < 0)
	{
		OutError = FString::Printf(TEXT("OrbCount must be >= 0 (got %d)"), Info.OrbCount);
		return false;
	}
	if (Info.TimeLimitSeconds < 10.f)
	{
		OutError = FString::Printf(TEXT("TimeLimitSeconds must be >= 10 (got %.1f)"), Info.TimeLimitSeconds);
		return false;
	}
	for (const FName& Key : Info.RequiredKeys)
	{
		if (Key.IsNone())
		{
			OutError = TEXT("RequiredKeys contains an empty entry");
			return false;
		}
	}
	OutError.Reset();
	return true;
}
