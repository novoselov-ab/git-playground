#pragma once
#include "ModuleInterface.h"

class FTurbulenceEditorModule : IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:
	TSharedPtr<IAssetTypeActions>	AttractorAssetTypeActions;
	TSharedPtr<IAssetTypeActions>	GridAssetTypeActions;
	TSharedPtr<IAssetTypeActions>	HeatSourceAssetTypeActions;
	TSharedPtr<IAssetTypeActions>	JetAssetTypeActions;
	TSharedPtr<IAssetTypeActions>	NoiseAssetTypeActions;
	TSharedPtr<IAssetTypeActions>	VelocitySourceAssetTypeActions;
	TSharedPtr<IAssetTypeActions>	VortexAssetTypeActions;

};