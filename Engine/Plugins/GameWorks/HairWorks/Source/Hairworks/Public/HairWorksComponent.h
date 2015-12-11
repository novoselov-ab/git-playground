#pragma once

#include "HairWorksInstance.h"
#include "HairWorksComponent.generated.h"

/**
* HairComponent manages and renders a hair asset.
*/
UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent), HideCategories = (Collision, Base, Object, PhysicsVolume))
class HAIRWORKS_API UHairWorksComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	UHairWorksComponent(const FObjectInitializer &ObjectInitializer);
	virtual ~UHairWorksComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hair, meta = (ShowOnlyInnerProperties))
	FHairWorksInstance HairInstance;

	FPrimitiveSceneProxy* CreateSceneProxy() override;
	void OnAttachmentChanged() override;
	void SendRenderDynamicData_Concurrent() override;
	void CreateRenderState_Concurrent() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

#if WITH_EDITOR
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

protected:
	/** Bone remapping */
	TArray<uint16> BoneIndices;

	// Bone names and indices.
	TMap<FName, uint16> HairBoneToIdxMap;

	// Send data for rendering
	void SendHairDynamicData();

	// Bone mapping
	void SetupBoneMapping();

};
