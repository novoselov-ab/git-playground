#pragma once

#include "AllowWindowsPlatformTypes.h"
#pragma warning(push)
#pragma warning(disable: 4191)	// For DLL function pointer conversion
#include "GFSDK_HairWorks.h"
#pragma warning(pop)
#include "HideWindowsPlatformTypes.h"

class FHairWorksLogger : public GFSDK_HAIR_LogHandler
{
	virtual void Log(GFSDK_HAIR_LOG_TYPES logType, const char* message, const char* file, int line) override;
};