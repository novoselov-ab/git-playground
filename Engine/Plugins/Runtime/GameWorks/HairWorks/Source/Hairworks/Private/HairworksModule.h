#pragma once

#include "ModuleInterface.h"

class IAssetTypeActions;
class FHairManager;
class GFSDK_HairSDK;

extern FHairManager* GHairManager;

class FHairworksModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	
private:
	TUniquePtr<FHairManager>		HairManager;
	TSharedPtr<IAssetTypeActions>	HairAssetTypeActions;
};

//////////////////////////////////////////////////////////////////////////

