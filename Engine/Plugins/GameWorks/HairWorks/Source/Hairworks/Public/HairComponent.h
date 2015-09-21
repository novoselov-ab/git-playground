#pragma once

#include "HairComponent.generated.h"

class UHair;

/**
* HairComponent manages and renders a hair asset.
*/
UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent), HideCategories = (Collision, Base, Object, PhysicsVolume))
class HAIRWORKS_API UHairComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	UHairComponent(const FObjectInitializer &ObjectInitializer);
	virtual ~UHairComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
	UHair* Hair;

	// This is a copy of the HairProperties taken from the original asset.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Hair)
	FHairProperties	HairProperties;


	FPrimitiveSceneProxy* CreateSceneProxy() override;
	void OnAttachmentChanged() override;
	void SendRenderDynamicData_Concurrent() override;
	void CreateRenderState_Concurrent() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

#if WITH_EDITOR

	//JDM: TODO: Add watchers for the textures so we can limit our calls to UpdateShaderCache - or just be dumb and any time one of its members changes we're invalid
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	const FHairProperties& GetProperties() { return HairProperties; }

protected:
	UPROPERTY()
	USkinnedMeshComponent* ParentSkeleton;

	// This is a cached copy of the hair descriptor that's updated every time HairProperties changes.
	GFSDK_HairInstanceDescriptor	CachedHairDescriptor;

	// Bone names and indices.
	TMap<FName, int32> HairBoneToIdxMap;

	// Send data for rendering
	void SendHairDynamicData();

	// Bone mapping
	void SetupBoneMapping();

};
