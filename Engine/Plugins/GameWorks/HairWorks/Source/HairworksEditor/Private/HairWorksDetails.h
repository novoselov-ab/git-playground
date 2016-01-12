#pragma once

class UHairWorksMaterial;

class FHairWorksMaterialDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

	static bool IsResetVisible(TSharedRef<IPropertyHandle> PropertyHandle, TSharedRef<TArray<TWeakObjectPtr<UHairWorksMaterial>>> SelectedObjects);
	static void ResetProperty(TSharedRef<IPropertyHandle> PropertyHandle, TSharedRef<TArray<TWeakObjectPtr<UHairWorksMaterial>>> SelectedObjects);
	static UHairWorksMaterial& GetDefaultHairMaterial(const UHairWorksMaterial& HairMaterial);
};
