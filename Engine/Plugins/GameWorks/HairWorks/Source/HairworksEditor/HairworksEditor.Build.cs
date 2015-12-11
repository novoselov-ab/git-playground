namespace UnrealBuildTool.Rules
{
	public class HairWorksEditor : ModuleRules
	{
		public HairWorksEditor(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"HairWorksEditor/Private",
					// ... add other private include paths required here ...
				}
				);

            PublicIncludePaths.AddRange(
                new string[] { 
                    "HairWorksEditor/Public"
                }
            );

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"HairWorks",
				    "Core",
				    "CoreUObject",
				    "AssetTools",
    				"Engine",
                    "UnrealEd",
					"RenderCore",
                    "Renderer",
				    "PropertyEditor"

				}
				);

            
		    PrivateIncludePathModuleNames.AddRange(
			    new string[] {
                    "HairWorks",
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