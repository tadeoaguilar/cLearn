#include "CLearn/CLPrimerLibrary.h"

#include "CLearn/CLLog.h"

void UCLPrimerLibrary::RunPrimerTour()
{
	// ---- Strings ----
	FString Name = TEXT("Ada");                                 // TEXT() makes a TCHAR literal
	FString Greeting = FString::Printf(TEXT("Hello, %s! You are level %d."), *Name, 7); // * gives a const TCHAR*
	UE_LOG(LogCLearn, Log, TEXT("%s"), *Greeting);
	UE_LOG(LogCLearn, Log, TEXT("Len=%d Upper=%s Contains 'level'=%d"), Greeting.Len(), *Greeting.ToUpper(),
	       Greeting.Contains(TEXT("level")));

	FString Formatted = FString::Format(TEXT("{0} has {1} orbs"), {Name, 42}); // positional formatting
	UE_LOG(LogCLearn, Log, TEXT("%s"), *Formatted);

	const FName Tag(TEXT("Pickup"));                            // FName: interned, case-insensitive compare
	UE_LOG(LogCLearn, Log, TEXT("FName equal ignoring case: %d"), Tag == FName(TEXT("pickup")));

	const FText Ui = NSLOCTEXT("CLearn", "Welcome", "Welcome to Orb Runner"); // localizable text for UI
	UE_LOG(LogCLearn, Log, TEXT("FText: %s"), *Ui.ToString());

	// ---- TArray (like std::vector) ----
	TArray<int32> Numbers = {5, 3, 8, 1};
	Numbers.Add(13);
	Numbers.Sort();                                             // 1 3 5 8 13
	Numbers.RemoveAll([](int32 N) { return N % 2 == 0; });      // lambdas work as predicates
	FString Joined = FString::JoinBy(Numbers, TEXT(", "), [](int32 N) { return FString::FromInt(N); });
	UE_LOG(LogCLearn, Log, TEXT("Odd numbers: %s (Num=%d)"), *Joined, Numbers.Num());
	if (const int32* Found = Numbers.FindByPredicate([](int32 N) { return N > 4; }))
	{
		UE_LOG(LogCLearn, Log, TEXT("First > 4: %d"), *Found);
	}

	// ---- TMap (like std::unordered_map) ----
	TMap<FName, int32> Inventory;
	Inventory.Add(TEXT("Orb"), 3);
	Inventory.FindOrAdd(TEXT("Key")) += 1;
	if (int32* Orbs = Inventory.Find(TEXT("Orb")))              // Find returns a pointer (nullptr if absent)
	{
		*Orbs += 2;
	}
	for (const TPair<FName, int32>& Pair : Inventory)
	{
		UE_LOG(LogCLearn, Log, TEXT("Inventory %s = %d"), *Pair.Key.ToString(), Pair.Value);
	}

	// ---- Assertions instead of exceptions ----
	ensureMsgf(Inventory.Contains(TEXT("Orb")), TEXT("ensure: logs + breaks in debugger once, but keeps running"));
	check(Numbers.Num() > 0); // check: fatal in development builds if false (compiled out in Shipping)

	UE_LOG(LogCLearn, Warning, TEXT("Primer tour finished — Warning lines show in yellow"));
}

int32 UCLPrimerLibrary::TotalPrice(const TArray<FCLItem>& Items)
{
	int32 Total = 0;
	for (const FCLItem& Item : Items)
	{
		Total += Item.Price;
	}
	return Total;
}

TArray<FCLItem> UCLPrimerLibrary::FilterByRarity(const TArray<FCLItem>& Items, ECLRarity Rarity)
{
	return Items.FilterByPredicate([Rarity](const FCLItem& Item) { return Item.Rarity == Rarity; });
}

bool UCLPrimerLibrary::FindItemById(const TArray<FCLItem>& Items, FName Id, FCLItem& OutItem)
{
	if (const FCLItem* Item = Items.FindByPredicate([Id](const FCLItem& I) { return I.Id == Id; }))
	{
		OutItem = *Item;
		return true;
	}
	return false;
}
