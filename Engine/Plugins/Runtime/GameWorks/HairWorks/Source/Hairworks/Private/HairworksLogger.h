#pragma once
#include "GFSDK_HairWorks.h"

class FHairWorksLogger : public GFSDK_HAIR_LogHandler
{
	virtual void Log(GFSDK_HAIR_LOG_TYPES logType, const char* message, const char* file, int line) override;
};