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
					"Core",
					"CoreUObject",
					"Engine",
					"RHI",
					"RenderCore",
					"Renderer",
					"ShaderCore",
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