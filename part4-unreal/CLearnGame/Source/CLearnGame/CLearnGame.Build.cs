// Module rules: what this C++ module depends on. UnrealBuildTool reads this C# file.
using UnrealBuildTool;

public class CLearnGame : ModuleRules
{
	public CLearnGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lets us write #include "CLearn/CLCharacter.h" from anywhere in the module.
		PublicIncludePaths.Add(ModuleDirectory);

		// Public: types from these modules appear in our headers.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"EnhancedInput",   // ch. 18: input actions & mapping contexts
			"UMG"              // ch. 20: UUserWidget in our headers
		});

		// Private: only used inside .cpp files.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate", "SlateCore",       // UMG's underlying UI framework
			"HTTP", "Json"              // ch. 20: quest log from the Tasks API
		});
	}
}
