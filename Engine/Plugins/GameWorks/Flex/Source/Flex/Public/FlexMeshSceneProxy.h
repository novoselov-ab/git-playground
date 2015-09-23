#pragma once
#include "StaticMeshResources.h"

/** Scene proxy, overrides default static mesh behavior */
class FFlexMeshSceneProxy : public FStaticMeshSceneProxy
{
public:

	FFlexMeshSceneProxy(UStaticMeshComponent* Component);
	virtual ~FFlexMeshSceneProxy();

	int GetLastVisibleFrame();

	void UpdateClothTransforms();
	void UpdateSoftTransforms(const FFlexShapeTransform* Transforms, int32 NumShapes);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) override;
	//virtual void PreRenderView(const FSceneViewFamily* ViewFamily, const uint32 VisibilityMap, int32 FrameNumber) override;
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override;

	virtual bool GetMeshElement(int32 LODIndex, int32 BatchIndex, int32 ElementIndex, uint8 InDepthPriorityGroup, const bool bUseSelectedMaterial, const bool bUseHoveredMaterial, FMeshBatch& OutMeshBatch) const;
	virtual bool GetShadowMeshElement(int32 LODIndex, int32 BatchIndex, uint8 InDepthPriorityGroup, FMeshBatch& OutMeshBatch, bool bDitheredLODTransition) const;

	FFlexVertexFactory* VertexFactory;

	UFlexComponent* FlexComponent;
	uint32 LastFrame;

	// shape transforms sent from game thread
	const FFlexShapeTransform* ShapeTransforms;
};