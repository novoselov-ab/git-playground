namespace UnrealBuildTool.Rules
{
	public class Flex : ModuleRules
	{
		public Flex(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"Flex/Private",
					// ... add other private include paths required here ...
				}
				);

            PublicIncludePaths.AddRange(
                new string[] { 
                    "Flex/Public"
                }
            );

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
//					"RHI",
// 					"RenderCore",
// 					"Renderer",
					"ShaderCore",
//     				"UnrealEd",
// 				    "AssetTools",
// 				    "ContentBrowser",
//                     "EditorStyle"
				}
				);

//             PublicDependencyModuleNames.AddRange(
//                 new string[] 
//                 {
//                 }
//             );

// 		    PrivateIncludePathModuleNames.AddRange(
// 			    new string[] {
// 				    "AssetTools",
// 				    "AssetRegistry"
// 			    });
// 
// 		    DynamicallyLoadedModuleNames.AddRange(
// 			    new string[] {
// 				    "AssetTools",
// 				    "AssetRegistry"
// 			    });

/*
            Definitions.Add("GW_HAIRWORKS=1");

            if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/HairWorks/Libraries/Win32/GFSDK_HairWorks.win32.dll"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/HairWorks/Libraries/Win64/GFSDK_HairWorks.win64.dll"));
            }
            // Add direct rendering dependencies on a per-platform basis
            if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
            {
                PrivateDependencyModuleNames.AddRange(new string[] { "D3D11RHI" });
                PrivateIncludePaths.AddRange(
                    new string[] {
  					    "../../../../Source/Runtime/Windows/D3D11RHI/Private",
  					    "../../../../Source/Runtime/Windows/D3D11RHI/Private/Windows",
					    // ... add other private include paths required here ...
    				    }
                    );
            }
			*/

		}
	}
}