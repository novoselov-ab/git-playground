#pragma once

#include "ModuleInterface.h"

class FHairWorksManager;
class GFSDK_HairSDK;

extern HAIRWORKS_API FHairWorksManager* GHairManager;

class FHairworksModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	
private:
	TUniquePtr<FHairWorksManager>		HairManager;
};

//////////////////////////////////////////////////////////////////////////

