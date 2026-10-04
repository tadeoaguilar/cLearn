#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CLLevelInfo.generated.h"

USTRUCT(BlueprintType)
struct FCLLevelInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FName LevelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FText Title;

	// The editor won't let designers type a negative value (ClampMin)...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level", meta = (ClampMin = "0"))
	int32 OrbCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level", meta = (ClampMin = "10", Units = "s"))
	float TimeLimitSeconds = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TArray<FName> RequiredKeys;
};

UCLASS()
class CLEARNGAME_API UCLLevelInfoLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ...but data can also come from code, JSON or Blueprint math, so validate at runtime too.
	UFUNCTION(BlueprintCallable, Category = "CLearn|Level")
	static bool ValidateLevelInfo(const FCLLevelInfo& Info, FString& OutError);
};
