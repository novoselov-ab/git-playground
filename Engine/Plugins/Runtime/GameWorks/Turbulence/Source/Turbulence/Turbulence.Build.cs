namespace UnrealBuildTool.Rules
{
	public class Turbulence : ModuleRules
	{
		public Turbulence(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"Turbulence/Private",
					// ... add other private include paths required here ...
				}
				);

            PublicIncludePaths.AddRange(
                new string[] { 
                    "Turbulence/Public"
                }
            );

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"RHI",
					"RenderCore",
					"Renderer",
					"ShaderCore",
				}
				);

            Definitions.Add("GW_TURBULENCE=1");

		}
	}
}