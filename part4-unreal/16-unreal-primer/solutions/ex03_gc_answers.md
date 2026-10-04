# Ex 3 — GC reasoning: answers

| Member | Safe? | Why / fix |
|--------|-------|-----------|
| (a) `UStaticMeshComponent* Mesh;` from `CreateDefaultSubobject` | ⚠️ works today, bad practice | The actor's component list keeps default subobjects alive, so it won't be collected. But it's invisible to the editor, Blueprints and serialization. **Fix:** `UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Mesh;` |
| (b) `UPROPERTY() TObjectPtr<AActor> Target;` | ✅ | GC-tracked. When the actor is destroyed, the GC nulls this reference. Still check `IsValid(Target)` before use: a destroyed actor may linger until the next GC pass. |
| (c) `TArray<AActor*> Visited;` | ❌ | The GC doesn't see these pointers, so they may dangle after an actor is destroyed. **Fix:** `UPROPERTY() TArray<TObjectPtr<AActor>> Visited;`, or `TArray<TWeakObjectPtr<AActor>>` if you don't want to keep them alive. |
| (d) `TSharedPtr<FJsonObject> Config;` | ✅ | `FJsonObject` isn't a UObject, so normal reference counting (like `std::shared_ptr`) applies. It can't be a `UPROPERTY`, and doesn't need to be. |
| (e) `TWeakObjectPtr<APawn> LastInstigator;` | ✅ | Weak by design: `.Get()` returns nullptr once the pawn is gone. It doesn't keep the pawn alive. |
| (f) `UMaterialInstanceDynamic* Mid;` created at runtime | ❌ | `NewObject`/`Create` results are only referenced by this raw pointer, so the GC can **delete it under you** (classic "material randomly turns default" bug). **Fix:** `UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Mid;` |

Rule of thumb: **every UObject pointer stored in a member is a `UPROPERTY()`**
(`TObjectPtr<T>` for strong, `TWeakObjectPtr<T>` for weak). Raw `T*` is fine for
locals and function parameters.
