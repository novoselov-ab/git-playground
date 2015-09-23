#pragma once
#include "FlexVertexFactory.h"
#include "FlexVertexBuffer.h"

// Overrides local vertex factory with CPU skinned deformation
class FFlexCPUVertexFactory : public FFlexVertexFactory
{
public:

	FFlexCPUVertexFactory(const FLocalVertexFactory& Base, int NumVerts);
	virtual ~FFlexCPUVertexFactory();

	virtual void SkinCloth(const FVector4* SimulatedPositions, const FVector* SimulatedNormals, const int* VertexToParticleMap) override;
	virtual void SkinSoft(const FPositionVertexBuffer& Positions, const FStaticMeshVertexBuffer& Vertices, const FFlexShapeTransform* Transforms, const FVector* RestPoses, const int16* ClusterIndices, const float* ClusterWeights, int NumClusters) override;

	// Stores CPU skinned positions and normals to override default static mesh stream
	FFlexVertexBuffer VertexBuffer;
};
