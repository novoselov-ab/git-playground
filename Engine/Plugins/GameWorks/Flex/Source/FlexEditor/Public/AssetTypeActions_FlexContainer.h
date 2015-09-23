// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_FlexContainer : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_FlexContainer(EAssetTypeCategories::Type InCategory);

	// IAssetTypeActions Implementation
	virtual FText GetName() const override;	
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;

private:
	EAssetTypeCategories::Type MyAssetCategory;

};