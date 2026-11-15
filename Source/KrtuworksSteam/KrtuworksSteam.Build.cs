using UnrealBuildTool;

// Steam for the Krtuworks SDK: a separate plugin, so a game without Steam never compiles against Steamworks.
public class KrtuworksSteam : ModuleRules
{
	public KrtuworksSteam(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// The Krtuworks core reports failures as exceptions (its platform services may be called inside them).
		bEnableExceptions = true;
		PrivateDependencyModuleNames.AddRange(new string[] { "Core", "Engine", "Krtuworks" });
		AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
	}
}
