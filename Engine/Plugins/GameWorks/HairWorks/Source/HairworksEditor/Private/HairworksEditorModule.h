#pragma once

#include "ModuleManager.h"

class IAssetTypeActions;

class FHairworksEditorModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	
private:
	TSharedPtr<IAssetTypeActions>	HairAssetTypeActions;
};

//////////////////////////////////////////////////////////////////////////

