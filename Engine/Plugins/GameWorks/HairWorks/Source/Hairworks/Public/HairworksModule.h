#pragma once

#include "ModuleInterface.h"

class FHairManager;
class GFSDK_HairSDK;

extern HAIRWORKS_API FHairManager* GHairManager;

class FHairworksModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	
private:
	TUniquePtr<FHairManager>		HairManager;
};

//////////////////////////////////////////////////////////////////////////

