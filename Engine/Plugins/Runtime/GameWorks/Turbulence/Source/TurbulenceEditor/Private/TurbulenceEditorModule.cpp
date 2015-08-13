#include "TurbulenceEditorPCH.h"

IMPLEMENT_MODULE(FTurbulenceEditorModule, TurbulenceEditor);


void FTurbulenceEditorModule::StartupModule()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	AttractorAssetTypeActions = MakeShareable<FAssetTypeActions_AttractorAsset>();
	GridAssetTypeActions = MakeShareable<FAssetTypeActions_GridAsset>();
	HeatSourceAssetTypeActions = MakeShareable<FAssetTypeActions_HeatSourceAsset>();
	JetAssetTypeActions = MakeShareable<FAssetTypeActions_JetAsset>();
	NoiseAssetTypeActions = MakeShareable<FAssetTypeActions_NoiseAsset>();
	VelocitySourceAssetTypeActions = MakeShareable<FAssetTypeActions_VelocitySourceAsset>();
	VortexAssetTypeActions = MakeShareable<FAssetTypeActions_VortexAsset>();

	AssetTools.RegisterAssetTypeActions(AttractorAssetTypeActions.ToSharedRef());
	AssetTools.RegisterAssetTypeActions(GridAssetTypeActions.ToSharedRef());
	AssetTools.RegisterAssetTypeActions(HeatSourceAssetTypeActions.ToSharedRef());
	AssetTools.RegisterAssetTypeActions(JetAssetTypeActions.ToSharedRef());
	AssetTools.RegisterAssetTypeActions(NoiseAssetTypeActions.ToSharedRef());
	AssetTools.RegisterAssetTypeActions(VelocitySourceAssetTypeActions.ToSharedRef());
	AssetTools.RegisterAssetTypeActions(VortexAssetTypeActions.ToSharedRef());
}

void FTurbulenceEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		
		if (AttractorAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(AttractorAssetTypeActions.ToSharedRef());
		}

		if (GridAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(GridAssetTypeActions.ToSharedRef());
		}

		if (HeatSourceAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(HeatSourceAssetTypeActions.ToSharedRef());
		}

		if (JetAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(JetAssetTypeActions.ToSharedRef());
		}

		if (NoiseAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(NoiseAssetTypeActions.ToSharedRef());
		}

		if (VelocitySourceAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(VelocitySourceAssetTypeActions.ToSharedRef());
		}

		if (VortexAssetTypeActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(VortexAssetTypeActions.ToSharedRef());
		}
	}
}

