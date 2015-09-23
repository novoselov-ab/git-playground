#include "FlexEditorPCH.h"

#include "FlexContainer.h"

FAssetTypeActions_FlexContainer::FAssetTypeActions_FlexContainer(EAssetTypeCategories::Type InCategory):
	MyAssetCategory(InCategory)
{

}

FText FAssetTypeActions_FlexContainer::GetName() const
{
	return NSLOCTEXT("FlexEditor", "FlexEditor_FlexContainer", "Flex Container");
}

FColor FAssetTypeActions_FlexContainer::GetTypeColor() const
{
	return FColor(164, 211, 129);
}

UClass* FAssetTypeActions_FlexContainer::GetSupportedClass() const
{
	return UFlexContainer::StaticClass();
}

uint32 FAssetTypeActions_FlexContainer::GetCategories()
{
	return MyAssetCategory;
}
