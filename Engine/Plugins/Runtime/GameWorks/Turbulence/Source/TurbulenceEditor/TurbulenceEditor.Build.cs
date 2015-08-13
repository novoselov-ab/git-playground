namespace UnrealBuildTool.Rules
{
	public class TurbulenceEditor : ModuleRules
	{
		public TurbulenceEditor(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"TurbulenceEditor/Private",
					"TurbulenceEditor/Private/ActorFactories",
					"TurbulenceEditor/Private/AssetFactories",
					"TurbulenceEditor/Private/AssetTypeActions",
					"TurbulenceEditor/Private/ComponentVisualizers",
					// ... add other private include paths required here ...
				}
				);

            PublicIncludePaths.AddRange(
                new string[] { 
                    "TurbulenceEditor/Public",
					"TurbulenceEditor/Public/ComponentVisualizers",
                }
            );

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "Turbulence",
					"Core",
					"CoreUObject",
					"Engine",
     				"UnrealEd",
 				    "AssetTools",
 				    "ContentBrowser",
                    "EditorStyle"
				}
				);

 		    PrivateIncludePathModuleNames.AddRange(
 			    new string[] {
 				    "AssetTools",
 				    "AssetRegistry"
 			    });
 
 		    DynamicallyLoadedModuleNames.AddRange(
 			    new string[] {
 				    "AssetTools",
 				    "AssetRegistry"
 			    });
		}
	}
}