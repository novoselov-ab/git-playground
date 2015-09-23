#pragma once
#include "LocalVertexFactory.h"
#include "StaticMeshResources.h"
#include "FlexRender.h"

// Flex vertex factories override the local vertex factory and modify the position stream
class FFlexVertexFactory : public FLocalVertexFactory
{
public:

	virtual void SkinCloth(const FVector4* SimulatedPositions, const FVector* SimulatedNormals, const int* VertexToParticleMap) = 0;
	virtual void SkinSoft(const FPositionVertexBuffer& Positions, const FStaticMeshVertexBuffer& Vertices, const FFlexShapeTransform* Transforms, const FVector* RestPoses, const int16* ClusterIndices, const float* ClusterWeights, int NumClusters) = 0;
};
