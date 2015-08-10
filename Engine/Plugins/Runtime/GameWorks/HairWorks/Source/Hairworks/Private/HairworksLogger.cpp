#include "HairworksPCH.h"

void FHairWorksLogger::Log(GFSDK_HAIR_LOG_TYPES logType, const char* message, const char* file, int line)
{
	UE_LOG(LogHairWorks, Log, TEXT("HWDLL: %s"), *FString(message));
}
