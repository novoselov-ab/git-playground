#pragma once

#include "Engine.h"

#include "AllowWindowsPlatformTypes.h"
#pragma warning(push)
#pragma warning(disable: 4191)	// For DLL function pointer conversion
#include "GFSDK_HairWorks.h"
#pragma warning(pop)
#include "HideWindowsPlatformTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHairWorks, Verbose, All);


#include "HairworksModule.h"
#include "Hair.h"
#include "HairComponent.h"
#include "HairSceneProxy.h"
#include "HairManager.h"
#include "HairworksLogger.h"
#include "HairShaders.h"


