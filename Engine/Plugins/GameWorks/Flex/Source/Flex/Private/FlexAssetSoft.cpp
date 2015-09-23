#include "FlexPCH.h"

/*=============================================================================
UFlexAssetSoft
=============================================================================*/

void FFlexSoftSkinningIndicesVertexBuffer::Init(const TArray<int32>& ClusterIndices)
{
	Vertices.SetNum(ClusterIndices.Num());

	// convert to 16bit indices (this means max of 65k bones per soft.. should be enough for anybody)
	for (int i = 0; i < ClusterIndices.Num(); ++i)
		Vertices[i] = int16(ClusterIndices[i]);
}

void FFlexSoftSkinningIndicesVertexBuffer::InitRHI()
{
	if (Vertices.Num() > 0)
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(int16), BUF_Static, CreateInfo);

		// Copy the vertex data into the vertex buffer.
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(int16), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(int16));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FFlexSoftSkinningWeightsVertexBuffer::Init(const TArray<float>& ClusterWeights)
{
	Vertices = ClusterWeights;
}

void FFlexSoftSkinningWeightsVertexBuffer::InitRHI()
{
	if (Vertices.Num() > 0)
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(float), BUF_Static, CreateInfo);

		// Copy the vertex data into the vertex buffer.
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(float), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(float));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

UFlexAssetSoft::UFlexAssetSoft(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Phase.IgnoreRestCollisions = true;

	ContainerTemplate = NULL;
	AttachToRigids = false;
	Mass = 1.0f;

	ParticleSpacing = 10.0f;
	VolumeSampling = 4.0f;
	SurfaceSampling = 1.0f;

	ClusterSpacing = 20.0f;
	ClusterRadius = 30.0f;
	ClusterStiffness = 0.5f;

	LinkRadius = 0.0f;
	LinkStiffness = 1.0f;

	SkinningFalloff = 2.0f;
	SkinningMaxDistance = 100.0f;
}

void UFlexAssetSoft::ReImport(const UStaticMesh* Mesh)
{

#if WITH_FLEX
	Particles.Empty();

	ShapeCenters.Empty();
	ShapeIndices.Empty();
	ShapeOffsets.Empty();
	ShapeCoefficients.Empty();

	SpringIndices.Empty();
	SpringCoefficients.Empty();
	SpringRestLengths.Empty();

	// create particles from mesh LOD0	
	if (Mesh->RenderData->LODResources.Num() == 0)
		return;

	FStaticMeshLODResources* Res = &Mesh->RenderData->LODResources[0];

	// flatten vertex struct to positions 
	TArray<FVector> Positions;
	int32 NumVertices = Res->PositionVertexBuffer.GetNumVertices();

	for (int i = 0; i < NumVertices; ++i)
		Positions.Add(Res->PositionVertexBuffer.VertexPosition(i));

	TArray<uint32> VertexIndices;
	Res->IndexBuffer.GetCopy(VertexIndices);

	UE_LOG(LogFlex, Display, TEXT("Voxelizing Flex rigid body\n"));

	FlexExtAsset* Asset = flexExtCreateSoftFromMesh(
		(const float*)&Positions[0],
		Positions.Num(),
		(const int*)&VertexIndices[0],
		VertexIndices.Num(),
		ParticleSpacing,
		VolumeSampling,
		SurfaceSampling,
		ClusterSpacing,
		ClusterRadius,
		ClusterStiffness,
		LinkRadius,
		LinkStiffness);

	if (Asset)
	{
		SkinningWeights.SetNum(Positions.Num() * 4);
		SkinningIndices.SetNum(Positions.Num() * 4);

		// create skinning
		flexExtCreateSoftMeshSkinning(
			(const float*)&Positions[0],
			Positions.Num(),
			Asset->mShapeCenters,
			Asset->mNumShapes,
			SkinningFalloff,
			SkinningMaxDistance,
			&SkinningWeights[0],
			&SkinningIndices[0]);

		const float InvMass = (Mass > 0.0f) ? (1.0f / Mass) : 0.0f;

		// create particles
		for (int i = 0; i < Asset->mNumParticles; ++i)
		{
			FVector4 Particle = ((FVector4*)Asset->mParticles)[i];
			Particle.W = InvMass;

			Particles.Add(Particle);
		}

		// create shapes
		if (Asset->mNumShapes)
		{
			ShapeCenters.Append((FVector*)Asset->mShapeCenters, Asset->mNumShapes);
			ShapeCoefficients.Append(Asset->mShapeCoefficients, Asset->mNumShapes);
			ShapeOffsets.Append(Asset->mShapeOffsets, Asset->mNumShapes);
			ShapeIndices.Append(Asset->mShapeIndices, Asset->mNumShapeIndices);
		}

		// create links
		if (Asset->mNumSprings)
		{
			SpringIndices.Append(Asset->mSpringIndices, Asset->mNumSprings * 2);
			SpringRestLengths.Append(Asset->mSpringRestLengths, Asset->mNumSprings);
			SpringCoefficients.Append(Asset->mSpringCoefficients, Asset->mNumSprings);
		}

		flexExtDestroyAsset(Asset);
	}
	else
	{
		UE_LOG(LogFlex, Warning, TEXT("Failed to voxelize Flex rigid, check mesh is closed and objectSize/SamplingDistance < 64\n"));
	}

	UE_LOG(LogFlex, Display, TEXT("Created a Flex soft body with %d Particles, %d Springs, %d Clusters\n"), Particles.Num(), SpringCoefficients.Num(), ShapeCenters.Num());

	// update vertex buffer data
	WeightsVertexBuffer.Init(SkinningWeights);
	IndicesVertexBuffer.Init(SkinningIndices);

	// initialize / update resources
	if (WeightsVertexBuffer.IsInitialized())
		BeginUpdateResourceRHI(&WeightsVertexBuffer);
	else
		BeginInitResource(&WeightsVertexBuffer);

	if (IndicesVertexBuffer.IsInitialized())
		BeginUpdateResourceRHI(&IndicesVertexBuffer);
	else
		BeginInitResource(&IndicesVertexBuffer);

#endif //WITH_FLEX
}

void UFlexAssetSoft::PostLoad()
{
	Super::PostLoad();

	// update vertex buffer data
	WeightsVertexBuffer.Init(SkinningWeights);
	IndicesVertexBuffer.Init(SkinningIndices);

	BeginInitResource(&WeightsVertexBuffer);
	BeginInitResource(&IndicesVertexBuffer);
}

void UFlexAssetSoft::BeginDestroy()
{
	Super::BeginDestroy();

	BeginReleaseResource(&WeightsVertexBuffer);
	BeginReleaseResource(&IndicesVertexBuffer);
}

FlexExtAsset* UFlexAssetSoft::GetFlexAsset()
{
#if WITH_FLEX

	// reset Asset
	FMemory::Memset(Asset, 0, sizeof(FlexExtAsset));

	// particles
	Asset->mNumParticles = Particles.Num();
	if (Asset->mNumParticles)
		Asset->mParticles = (float*)&Particles[0];

	// distance constraints
	Asset->mNumSprings = SpringCoefficients.Num();
	if (Asset->mNumSprings)
	{
		Asset->mSpringIndices = (int*)&SpringIndices[0];
		Asset->mSpringCoefficients = (float*)&SpringCoefficients[0];
		Asset->mSpringRestLengths = (float*)&SpringRestLengths[0];
	}

	// soft bodies
	Asset->mNumShapes = ShapeCenters.Num();
	if (Asset->mNumShapes)
	{
		Asset->mNumShapeIndices = ShapeIndices.Num();
		Asset->mShapeOffsets = &ShapeOffsets[0];
		Asset->mShapeIndices = &ShapeIndices[0];
		Asset->mShapeCoefficients = &ShapeCoefficients[0];
		Asset->mShapeCenters = (float*)&ShapeCenters[0];
	}

#endif //WITH_FLEX

	return Asset;
}
