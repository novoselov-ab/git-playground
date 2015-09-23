#include "FlexEditorPCH.h"

IMPLEMENT_MODULE( FFlexEditorModule, FlexEditor );
DEFINE_LOG_CATEGORY(LogFlexEditor);

void FFlexEditorModule::StartupModule()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	FlexAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Flex")), NSLOCTEXT("FlexEditor", "FlexAssetCategory", "Flex"));

	FlexContainerAssetTypeAction = MakeShareable(new FAssetTypeActions_FlexContainer(FlexAssetCategoryBit));
	AssetTools.RegisterAssetTypeActions(FlexContainerAssetTypeAction.ToSharedRef());
// 
// 	FlexFluidSurfaceAssetTypeAction = MakeShareable(new FAssetTypeActions_FlexFluidSurface);
// 	AssetTools.RegisterAssetTypeActions(FlexFluidSurfaceAssetTypeAction.ToSharedRef());
}

void FFlexEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		if (FlexContainerAssetTypeAction.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(FlexContainerAssetTypeAction.ToSharedRef());
		}
// 
// 		if (FlexFluidSurfaceAssetTypeAction.IsValid())
// 		{
// 			AssetTools.UnregisterAssetTypeActions(FlexFluidSurfaceAssetTypeAction.ToSharedRef());
// 		}
	}
}
