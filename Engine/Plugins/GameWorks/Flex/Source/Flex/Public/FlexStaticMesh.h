#pragma once

#include "FlexStaticMesh.generated.h"

UCLASS(collapsecategories, hidecategories = Object, customconstructor, MinimalAPI, BlueprintType, config = Engine)
class UFlexStaticMesh : public UStaticMesh
{
	GENERATED_BODY()
public:

	/** Properties for the associated Flex object */
	UPROPERTY(EditAnywhere, Instanced, Category = Flex)
	class UFlexAsset* FlexAsset;

};