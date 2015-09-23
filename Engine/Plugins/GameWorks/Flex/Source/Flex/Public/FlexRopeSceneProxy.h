#pragma once
#include "RenderResource.h"
#include "LocalVertexFactory.h"
#include "PrimitiveSceneProxy.h"


/** Vertex Buffer */
class FFlexRopeVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI() override;

	int32 NumVerts;
};

/** Index Buffer */
class FFlexRopeIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI() override;

	int32 NumIndices;
};

/** Vertex Factory */
class FFlexRopeVertexFactory : public FLocalVertexFactory
{
public:

	FFlexRopeVertexFactory() = default;


	/** Initialization */
	void Init(const FFlexRopeVertexBuffer* VertexBuffer);
};

/** Dynamic data sent to render thread */
struct FFlexRopeDynamicData
{
	/** Array of points */
	TArray<FVector> FlexRopePoints;
};


//////////////////////////////////////////////////////////////////////////
// FFlexRopeSceneProxy

class FFlexRopeSceneProxy : public FPrimitiveSceneProxy
{
public:

	FFlexRopeSceneProxy(UFlexRopeComponent* Component);

	virtual ~FFlexRopeSceneProxy();

	int32 GetRequiredVertexCount() const;

	int32 GetRequiredIndexCount() const;

	int32 GetVertIndex(int32 AlongIdx, int32 AroundIdx) const;

	void BuildRopeMesh(const TArray<FVector>& InPoints, TArray<FDynamicMeshVertex>& OutVertices, TArray<int32>& OutIndices);

	/** Called on render thread to assign new dynamic data */
	void SetDynamicData_RenderThread(FFlexRopeDynamicData* NewDynamicData);

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	virtual uint32 GetMemoryFootprint(void) const;

	uint32 GetAllocatedSize(void) const;

private:

	UMaterialInterface* Material;

	FFlexRopeVertexBuffer VertexBuffer;
	FFlexRopeIndexBuffer IndexBuffer;
	FFlexRopeVertexFactory VertexFactory;

	FFlexRopeDynamicData* DynamicData;

	FMaterialRelevance MaterialRelevance;

	int32 NumSegments;

	float Width;

	int32 NumSides;

	float TileMaterial;
};