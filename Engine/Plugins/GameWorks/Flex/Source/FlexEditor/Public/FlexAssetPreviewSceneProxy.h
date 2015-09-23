#pragma once

#include "PrimitiveSceneProxy.h"

class FFlexAssetPreviewSceneProxy : public FPrimitiveSceneProxy
{
	struct Line
	{
		FVector Start;
		FVector End;
		FColor Color;
	};

public:

	FFlexAssetPreviewSceneProxy(const UFlexAssetPreviewComponent* InComponent);

	void AddSolidSphere(FVector Position, float Radius, FColor Color, int32 NumSides, int32 NumRings);

	void AddLine(FVector Start, FVector End, FColor Color);

	void AddBasis(FVector Position, float Length);


	//virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View) override
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	virtual uint32 GetMemoryFootprint(void) const;

	uint32 GetAllocatedSize(void) const;

private:

	TArray<FDynamicMeshVertex> Vertices;
	TArray<int32> Triangles;

	TArray<Line> Lines;

	FPrimitiveViewRelevance ViewRelevance;
};