#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CLWordLibrary.generated.h"

UCLASS()
class CLEARNGAME_API UCLWordLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Case-insensitive word counts; words are separated by whitespace, punctuation is stripped.
	UFUNCTION(BlueprintPure, Category = "CLearn|Words")
	static TMap<FString, int32> CountWords(const FString& Text);

	// The N most frequent words (ties broken alphabetically).
	UFUNCTION(BlueprintPure, Category = "CLearn|Words")
	static TArray<FString> TopWords(const FString& Text, int32 N);
};
