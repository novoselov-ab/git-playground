#include "FlexPCH.h"

/*=============================================================================
UFlexAssetSolid
=============================================================================*/

UFlexAssetSolid::UFlexAssetSolid(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ContainerTemplate = NULL;
	AttachToRigids = false;
	Mass = 1.0f;

	Stiffness = 1.0f;
	SamplingDistance = 10.0f;
}

void UFlexAssetSolid::ReImport(const UStaticMesh* Mesh)
{
#if WITH_FLEX
	Particles.Empty();
	ShapeCenters.Empty();
	ShapeIndices.Empty();
	ShapeOffsets.Empty();
	ShapeCoefficients.Empty();

	// create particles from mesh LOD0	
	if (Mesh->RenderData->LODResources.Num() == 0)
		return;

	FStaticMeshLODResources* Res = &Mesh->RenderData->LODResources[0];

	// flatten vertex struct to positions 
	TArray<FVector> Positions;

	int32 NumVertices = Res->PositionVertexBuffer.GetNumVertices();

	float InvMass = (Mass > 0.0f) ? (1.0f / Mass) : 0.0f;

	for (int i = 0; i < NumVertices; ++i)
	{
		FVector Pos = Res->PositionVertexBuffer.VertexPosition(i);
		Positions.Add(Pos);
	}

	TArray<uint32> VertexIndices;
	Res->IndexBuffer.GetCopy(VertexIndices);

	UE_LOG(LogFlex, Display, TEXT("Voxelizing Flex rigid body\n"));

	FlexExtAsset* Asset = flexExtCreateRigidFromMesh((const float*)&Positions[0], Positions.Num(), (const int*)&VertexIndices[0], VertexIndices.Num(), SamplingDistance, 0.0f);

	if (Asset)
	{
		for (int i = 0; i < Asset->mNumParticles; ++i)
		{
			FVector4 Particle = ((FVector4*)Asset->mParticles)[i];
			Particle.W = InvMass;

			Particles.Add(Particle);
		}

		for (int i = 0; i < Asset->mNumShapes; ++i)
		{
			ShapeCenters.Add(((FVector*)Asset->mShapeCenters)[i]);
			ShapeOffsets.Add(Asset->mShapeOffsets[i]);
			ShapeCoefficients.Add(Asset->mShapeCoefficients[i]);
		}

		for (int i = 0; i < Asset->mNumShapeIndices; ++i)
			ShapeIndices.Add(Asset->mShapeIndices[i]);

		flexExtDestroyAsset(Asset);
	}
	else
	{
		UE_LOG(LogFlex, Warning, TEXT("Failed to voxelize Flex rigid, check mesh is closed and objectSize/SamplingDistance < 64\n"));
	}

	UE_LOG(LogFlex, Display, TEXT("Created a FlexAsset with %d Particles, %d Springs, %d Triangles\n"), Particles.Num(), 0, 0);
#endif //WITH_FLEX
}

FlexExtAsset* UFlexAssetSolid::GetFlexAsset()
{
#if WITH_FLEX
	// reset Asset
	FMemory::Memset(Asset, 0, sizeof(FlexExtAsset));

	// re-update the Asset each time it is requested, could cache this
	Asset->mNumParticles = Particles.Num();

	// particles
	if (Asset->mNumParticles)
	{
		Asset->mParticles = (float*)&Particles[0];
	}

	if (ShapeCenters.Num() == 0)
	{
		// construct a single rigid shape constraint  
		ShapeCenters.Add(RigidCenter);
		ShapeCoefficients.Add(Stiffness);

		for (int i = 0; i < Particles.Num(); ++i)
			ShapeIndices.Add(i);

		ShapeOffsets.Add(Particles.Num());
	}

	// shapes 
	Asset->mNumShapes = ShapeCenters.Num();
	Asset->mNumShapeIndices = ShapeIndices.Num();
	Asset->mShapeOffsets = &ShapeOffsets[0];
	Asset->mShapeIndices = &ShapeIndices[0];
	Asset->mShapeCoefficients = &ShapeCoefficients[0];
	Asset->mShapeCenters = (float*)&ShapeCenters[0];

#endif //WITH_FLEX
	return Asset;
}