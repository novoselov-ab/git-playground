#pragma once

#include "ModuleInterface.h"

class IAssetTypeActions;

class FHairworksEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	
private:
	TSharedPtr<IAssetTypeActions>	HairAssetTypeActions;
};

//////////////////////////////////////////////////////////////////////////

