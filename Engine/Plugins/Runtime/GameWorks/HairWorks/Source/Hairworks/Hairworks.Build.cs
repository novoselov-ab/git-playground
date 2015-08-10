namespace UnrealBuildTool.Rules
{
	public class HairWorks : ModuleRules
	{
		public HairWorks(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"HairWorks/Private",
					"../../../../../Source/Runtime/Engine/Private",
 					"../../../../../Source/Runtime/Renderer/Private",
					// ... add other private include paths required here ...
				}
				);

            PublicIncludePaths.AddRange(
                new string[] { 
                    "HairWorks/Public"
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

            PublicDependencyModuleNames.AddRange(
                new string[] 
                {
					"RenderCore",
					"Renderer"
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

            Definitions.Add("GW_HAIRWORKS=1");


            // Add direct rendering dependencies on a per-platform basis
            if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
            {
                PrivateDependencyModuleNames.AddRange(new string[] { "D3D11RHI" });
                PrivateIncludePaths.AddRange(
                    new string[] {
  					    "../../../../../Source/Runtime/Windows/D3D11RHI/Private",
  					    "../../../../../Source/Runtime/Windows/D3D11RHI/Private/Windows",
					    // ... add other private include paths required here ...
    				    }
                    );
            }
		}
	}
}