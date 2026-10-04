#include "CLWordLibrary.h"

TMap<FString, int32> UCLWordLibrary::CountWords(const FString& Text)
{
	TArray<FString> Words;
	Text.ToLower().ParseIntoArrayWS(Words); // split on whitespace, drop empties

	TMap<FString, int32> Counts;
	for (FString& Word : Words)
	{
		// Strip leading/trailing punctuation: keep letters, digits and apostrophes
		int32 Start = 0, End = Word.Len();
		while (Start < End && !(FChar::IsAlnum(Word[Start]) || Word[Start] == TEXT('\''))) { ++Start; }
		while (End > Start && !(FChar::IsAlnum(Word[End - 1]) || Word[End - 1] == TEXT('\''))) { --End; }
		if (End > Start)
		{
			Counts.FindOrAdd(Word.Mid(Start, End - Start)) += 1; // FindOrAdd ≈ std::map::operator[]
		}
	}
	return Counts;
}

TArray<FString> UCLWordLibrary::TopWords(const FString& Text, int32 N)
{
	TMap<FString, int32> Counts = CountWords(Text);

	TArray<TPair<FString, int32>> Entries;
	for (const TPair<FString, int32>& Pair : Counts)
	{
		Entries.Add(Pair);
	}
	Entries.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B)
	{
		return A.Value != B.Value ? A.Value > B.Value : A.Key < B.Key;
	});

	TArray<FString> Result;
	for (int32 i = 0; i < FMath::Min(N, Entries.Num()); ++i)
	{
		Result.Add(Entries[i].Key);
	}
	return Result;
}
