namespace UnrealBuildTool.Rules
{
	public class FlexEditor : ModuleRules
	{
		public FlexEditor(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"FlexEditor/Private",
					// ... add other private include paths required here ...
				}
				);

            PublicIncludePaths.AddRange(
                new string[] { 
                    "FlexEditor/Public"
                }
            );

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Flex",
				    "Core",
				    "CoreUObject",
				    "AssetTools",
    				"Engine",
                    "UnrealEd",
					"RenderCore",
                    "Renderer"
				}
				);

            
		    PrivateIncludePathModuleNames.AddRange(
			    new string[] {
                    "Flex",
// 				    "AssetTools",
// 				    "AssetRegistry"
			    });
/*

		    DynamicallyLoadedModuleNames.AddRange(
			    new string[] {
				    "AssetTools",
				    "AssetRegistry"
			    });

            Definitions.Add("GW_HAIRWORKS=1");
*/

		}
	}
}