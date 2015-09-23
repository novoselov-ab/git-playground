#pragma once

#include "ModuleManager.h"
#include "AssetTypeCategories.h"

class IAssetTypeActions;

class FFlexEditorModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:
	EAssetTypeCategories::Type		FlexAssetCategoryBit;

	TSharedPtr<IAssetTypeActions>	FlexContainerAssetTypeAction;
//	TSharedPtr<IAssetTypeActions>	FlexFluidSurfaceAssetTypeAction;
};

//////////////////////////////////////////////////////////////////////////

