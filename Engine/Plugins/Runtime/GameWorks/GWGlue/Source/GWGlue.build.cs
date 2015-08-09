// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GWGlue : ModuleRules
{
	public GWGlue(TargetInfo Target)
	{
		PublicIncludePaths.Add("GWGlue/Public");
		PrivateIncludePaths.Add("GWGlue/Private");
	}
}
