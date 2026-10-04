# 16 — Exercises: Unreal Primer

Put your solutions in `Source/CLearnGame/CLearn/`, rebuild, and test them from
the Level Blueprint. Reference solutions are in `../solutions/`.

### Ex 1 — Translate standard C++ to Unreal ⭐
Rewrite this ch. 6 snippet using Unreal types and a `UBlueprintFunctionLibrary`
function `static TMap<FString, int32> CountWords(const FString& Text)`:
```cpp
std::unordered_map<std::string, int> counts;
std::istringstream in(text);
for (std::string w; in >> w;) ++counts[w];
```
Hints: `Text.ParseIntoArrayWS(Words)`, `TMap::FindOrAdd`, `FString::ToLower()`.
Then add `static TArray<FString> TopWords(const FString& Text, int32 N)`, using
`TArray::Sort` with a lambda.
→ `solutions/CLWordLibrary.h/.cpp`

### Ex 2 — A reflected data struct ⭐⭐
Create `USTRUCT(BlueprintType) FCLLevelInfo` with `FName LevelId`, `FText Title`,
`int32 OrbCount` (min 0), `float TimeLimitSeconds` (min 10, default 120) and
`TArray<FName> RequiredKeys`. Add a function-library function
`static bool ValidateLevelInfo(const FCLLevelInfo& Info, FString& OutError)`
that returns false and explains the first problem it finds: an empty id, a
negative orb count, or a time limit that is too short.
Expose it in a Blueprint and see how editor `meta` specifiers (`ClampMin`)
complement runtime validation.
→ `solutions/CLLevelInfo.h/.cpp`

### Ex 3 — GC reasoning (on paper) ⭐⭐
For each member below, say whether it's safe and why, then fix the unsafe ones:
```cpp
UCLASS() class ACLExample : public AActor {
    GENERATED_BODY()
    UStaticMeshComponent* Mesh;                    // (a) created with CreateDefaultSubobject
    UPROPERTY() TObjectPtr<AActor> Target;          // (b)
    TArray<AActor*> Visited;                        // (c)
    TSharedPtr<FJsonObject> Config;                 // (d)
    TWeakObjectPtr<APawn> LastInstigator;           // (e)
    UMaterialInstanceDynamic* Mid;                  // (f) created at runtime with UMaterialInstanceDynamic::Create
};
```
→ `solutions/ex03_gc_answers.md`
