#include "FlexPCH.h"

/** Scene proxy */
FFlexMeshSceneProxy::FFlexMeshSceneProxy(UStaticMeshComponent* Component)
	: FStaticMeshSceneProxy(Component)
{
	FlexComponent = Cast<UFlexComponent>(Component);

	const FStaticMeshLODResources& LOD = Component->StaticMesh->RenderData->LODResources[0];
	const UFlexAssetSoft* SoftAsset = Cast<UFlexAssetSoft>(FlexComponent->FlexAsset);

	ERHIFeatureLevel::Type FeatureLevel = Component->GetWorld()->FeatureLevel;

	if (FeatureLevel >= ERHIFeatureLevel::SM4 && SoftAsset)
	{
		// Ensure top LOD has a compatible material 
		FLODInfo& ProxyLODInfo = LODs[0];
		for (int i = 0; i < ProxyLODInfo.Sections.Num(); ++i)
		{
			if (!ProxyLODInfo.Sections[i].Material->CheckMaterialUsage(MATUSAGE_FlexMeshes))
				ProxyLODInfo.Sections[i].Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}

		// use GPU skinning for SM4 and soft assets only
		VertexFactory = new FFlexGPUVertexFactory(LOD.VertexFactory, &SoftAsset->WeightsVertexBuffer, &SoftAsset->IndicesVertexBuffer);
	}
	else
	{
		// use CPU skinning for everything eles
		VertexFactory = new FFlexCPUVertexFactory(LOD.VertexFactory, LOD.VertexBuffer.GetNumVertices());
	}

	ShapeTransforms = NULL;
	LastFrame = 0;
}

FFlexMeshSceneProxy::~FFlexMeshSceneProxy()
{
	check(IsInRenderingThread());

	delete VertexFactory;

	delete[] ShapeTransforms;
}

int FFlexMeshSceneProxy::GetLastVisibleFrame()
{
	// called by the game thread to determine whether to disable simulation
	return LastFrame;
}

FPrimitiveViewRelevance FFlexMeshSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance Relevance = FStaticMeshSceneProxy::GetViewRelevance(View);
	Relevance.bDynamicRelevance = true;
	Relevance.bStaticRelevance = false;

	return Relevance;
}

void FFlexMeshSceneProxy::UpdateClothTransforms()
{
	// unsafe: update vertex buffers here by grabbing data directly from simulation container 
	// this won't be necessary when skinning is done on the GPU
	VertexFactory->SkinCloth(
		&FlexComponent->SimPositions[0],
		&FlexComponent->SimNormals[0],
		&FlexComponent->FlexAsset->VertexToParticleMap[0]);
}

void FFlexMeshSceneProxy::UpdateSoftTransforms(const FFlexShapeTransform* NewTransforms, int32 NumShapes)
{
	// delete old transforms
	delete[] ShapeTransforms;
	ShapeTransforms = NewTransforms;

	const UFlexAssetSoft* SoftAsset = Cast<UFlexAssetSoft>(FlexComponent->FlexAsset);

	const FPositionVertexBuffer& Positions = FlexComponent->StaticMesh->RenderData->LODResources[0].PositionVertexBuffer;
	const FStaticMeshVertexBuffer& Vertices = FlexComponent->StaticMesh->RenderData->LODResources[0].VertexBuffer;

	// only used for CPU skinning
	const int16* ClusterIndices = &SoftAsset->IndicesVertexBuffer.Vertices[0];
	const float* ClusterWeights = &SoftAsset->WeightsVertexBuffer.Vertices[0];
	const FVector* RestPoses = &SoftAsset->ShapeCenters[0];

	VertexFactory->SkinSoft(Positions, Vertices, ShapeTransforms, RestPoses, ClusterIndices, ClusterWeights, NumShapes);
}

void FFlexMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const
{
	// store last renderer frame (used for LOD), is the const_cast valid here?
	const_cast<FFlexMeshSceneProxy*>(this)->LastFrame = GFrameNumber;

	FStaticMeshSceneProxy::GetDynamicMeshElements(Views, ViewFamily, VisibilityMap, Collector);
}

bool FFlexMeshSceneProxy::GetMeshElement(int32 LODIndex, int32 BatchIndex, int32 ElementIndex, uint8 InDepthPriorityGroup, const bool bUseSelectedMaterial, const bool bUseHoveredMaterial, FMeshBatch& OutMeshBatch) const
{
	bool Ret = FStaticMeshSceneProxy::GetMeshElement(LODIndex, BatchIndex, ElementIndex, InDepthPriorityGroup, bUseSelectedMaterial, bUseHoveredMaterial, OutMeshBatch);

	// override top LOD with our simulated vertex factory
	if (LODIndex == 0)
		OutMeshBatch.VertexFactory = VertexFactory;

	return Ret;
}

bool FFlexMeshSceneProxy::GetShadowMeshElement(int32 LODIndex, int32 BatchIndex, uint8 InDepthPriorityGroup, FMeshBatch& OutMeshBatch, bool bDitheredLODTransition) const
{
	return false;
}
