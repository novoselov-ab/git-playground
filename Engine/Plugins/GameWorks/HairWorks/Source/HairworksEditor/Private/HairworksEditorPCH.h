#pragma once

#include "Engine.h"

#include "AllowWindowsPlatformTypes.h"
#pragma warning(push)
#pragma warning(disable: 4191)	// For DLL function pointer conversion
#include "GFSDK_HairWorks.h"
#pragma warning(pop)
#include "HideWindowsPlatformTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHairWorksEditor, Verbose, All);

#include "HairworksEditorModule.h"
#include "AssetTypeActions_Hair.h"
#include "HairWorksFactory.h"
#include "HairWorksDetails.h"


