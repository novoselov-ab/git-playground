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
                    "PhysX",
                    "Apex",
//					"RHI",
// 					"RenderCore",
// 					"Renderer",
					"ShaderCore",
//     				"UnrealEd",
 				    "AssetTools",
// 				    "ContentBrowser",
//                     "EditorStyle"
				}
				);

//             PublicDependencyModuleNames.AddRange(
//                 new string[] 
//                 {
//                 }
//             );

		    PrivateIncludePathModuleNames.AddRange(
			    new string[] {
				    "AssetTools",
				    "AssetRegistry"
			    });

/* 		    DynamicallyLoadedModuleNames.AddRange(
			    new string[] {
				    "AssetTools",
				    "AssetRegistry"
			    });
 */

            // Add the 3 dlls to RuntimeDependencies so that they are copied when packaging.
            if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win32/cudart32_70.dll"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win64/cudart64_70.dll"));
            }


            if (Target.Configuration == UnrealTargetConfiguration.Debug && BuildConfiguration.bDebugBuildsActuallyUseDebugCRT)
            {
                if (Target.Platform == UnrealTargetPlatform.Win32)
                {
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win32/flexDebug_x86.dll"));
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win32/flexExtDebug_x86.dll"));
                }
                else if (Target.Platform == UnrealTargetPlatform.Win64)
                {
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win64/flexDebug_x64.dll"));
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win64/flexExtDebug_x64.dll"));
                }
            } else if (Target.Configuration == UnrealTargetConfiguration.Shipping || Target.Configuration == UnrealTargetConfiguration.Test)
            {
                if (Target.Platform == UnrealTargetPlatform.Win32)
                {
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win32/flexRelease_x86.dll"));
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win32/flexExtRelease_x86.dll"));
                }
                else if (Target.Platform == UnrealTargetPlatform.Win64)
                {
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win64/flexRelease_x64.dll"));
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win64/flexExtRelease_x64.dll"));
                }
            } else
            {
                if (Target.Platform == UnrealTargetPlatform.Win32)
                {
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win32/flexRelease_x86.dll"));
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win32/flexExtRelease_x86.dll"));
                }
                else if (Target.Platform == UnrealTargetPlatform.Win64)
                {
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win64/flexRelease_x64.dll"));
                    RuntimeDependencies.Add(new RuntimeDependency("$(EngineDir)/Plugins/GameWorks/Flex/Libraries/Win64/flexExtRelease_x64.dll"));
                }
            }

		}
	}
}