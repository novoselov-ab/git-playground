#include "FlexPCH.h"

/*=============================================================================
UFlexAssetCloth
=============================================================================*/

UFlexAssetCloth::UFlexAssetCloth(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ContainerTemplate = NULL;
	AttachToRigids = false;
	Mass = 1.0f;

	RigidStiffness = 0.0f;

	StretchStiffness = 1.0f;
	BendStiffness = 1.0f;
	TetherStiffness = 1.0f;
	TetherGive = 0.1f;
	EnableInflatable = false;
	OverPressure = 1.0f;
	InflatableStiffness = 1.0f;
}

void UFlexAssetCloth::ReImport(const UStaticMesh* Mesh)
{
#if WITH_FLEX
	Particles.Empty();

	SpringIndices.Empty();
	SpringCoefficients.Empty();
	SpringRestLengths.Empty();

	ShapeCenters.Empty();
	ShapeIndices.Empty();
	ShapeOffsets.Empty();
	ShapeCoefficients.Empty();

	Triangles.Empty();

	// create particles from mesh LOD0	
	if (Mesh->RenderData->LODResources.Num() == 0)
		return;

	FStaticMeshLODResources* Res = &Mesh->RenderData->LODResources[0];

	// flatten vertex struct to positions 
	TArray<FVector> Positions;

	int32 NumVertices = Res->PositionVertexBuffer.GetNumVertices();
	int32 NumColors = Res->ColorVertexBuffer.GetNumVertices();

	float InvMass = (Mass > 0.0f) ? (1.0f / Mass) : 0.0f;

	for (int i = 0; i < NumVertices; ++i)
	{
		FVector Pos = Res->PositionVertexBuffer.VertexPosition(i);
		Positions.Add(Pos);
	}

	TArray<uint32> VertexIndices;
	Res->IndexBuffer.GetCopy(VertexIndices);

	TArray<int> UniqueVerts;
	UniqueVerts.SetNum(NumVertices);

	// mapping from vertex index to particle index
	VertexToParticleMap.SetNum(NumVertices);

	// render mesh has vertex duplicates (for texture mapping etc), weld mesh and generate particles for unique verts
	int ParticleCount = flexExtCreateWeldedMeshIndices((float*)&Positions[0], NumVertices, &UniqueVerts[0], &VertexToParticleMap[0], THRESH_POINTS_ARE_SAME);

	FVector Center(0.0f);

	for (int i = 0; i < ParticleCount; ++i)
	{
		float MassScale = 1.0f;

		int VertIndex = UniqueVerts[i];
		if (VertIndex < NumColors)
		{
			// if there is a color channel set invmass according to color (zero = heavier)
			FColor Col = Res->ColorVertexBuffer.VertexColor(VertIndex);
			MassScale = Col.R / 255.0f;
		}

		FVector Pos = Positions[UniqueVerts[i]];

		Particles.Add(FVector4(Pos, InvMass*MassScale));

		Center += Pos;
	}

	Center /= float(ParticleCount);

	// remap render index buffer from vertices to particles
	TArray<int> ParticleIndices;
	for (int i = 0; i < VertexIndices.Num(); ++i)
		ParticleIndices.Add(VertexToParticleMap[VertexIndices[i]]);

	// create cloth from unique particles
	FlexExtAsset* Asset = flexExtCreateClothFromMesh((float*)&Particles[0], Particles.Num(), (int*)&ParticleIndices[0], ParticleIndices.Num() / 3, StretchStiffness, BendStiffness, TetherStiffness, TetherGive, OverPressure);

	if (Asset)
	{
		RigidCenter = Center;

		// copy out spring data
		for (int i = 0; i < Asset->mNumSprings; ++i)
		{
			SpringIndices.Add(Asset->mSpringIndices[i * 2 + 0]);
			SpringIndices.Add(Asset->mSpringIndices[i * 2 + 1]);
			SpringCoefficients.Add(Asset->mSpringCoefficients[i]);
			SpringRestLengths.Add(Asset->mSpringRestLengths[i]);
		}

		// faces for cloth
		for (int i = 0; i < Asset->mNumTriangles * 3; ++i)
			Triangles.Add(Asset->mTriangleIndices[i]);

		// save inflatable properties
		InflatableVolume = Asset->mInflatableVolume;
		InflatableStiffness = Asset->mInflatableStiffness;

		// discard flex asset, we will recreate it from our internal data
		flexExtDestroyAsset(Asset);
	}

	UE_LOG(LogFlex, Display, TEXT("Created a FlexAsset with %d Particles, %d Springs, %d Triangles\n"), Particles.Num(), SpringRestLengths.Num(), Triangles.Num() / 3);

#endif //WITH_FLEX
}

FlexExtAsset* UFlexAssetCloth::GetFlexAsset()
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

	// distance constraints
	Asset->mNumSprings = SpringCoefficients.Num();
	if (Asset->mNumSprings)
	{
		Asset->mSpringIndices = (int*)&SpringIndices[0];
		Asset->mSpringCoefficients = (float*)&SpringCoefficients[0];
		Asset->mSpringRestLengths = (float*)&SpringRestLengths[0];
	}

	// triangles
	Asset->mNumTriangles = Triangles.Num() / 3;
	if (Asset->mNumTriangles)
	{
		Asset->mTriangleIndices = (int*)&Triangles[0];
	}

	// inflatables
	Asset->mInflatable = EnableInflatable;
	Asset->mInflatablePressure = OverPressure;
	Asset->mInflatableVolume = InflatableVolume;
	Asset->mInflatableStiffness = InflatableStiffness;

	if (RigidStiffness > 0.0f && ShapeCenters.Num() == 0)
	{
		// construct a single rigid shape constraint  
		ShapeCenters.Add(RigidCenter);
		ShapeCoefficients.Add(RigidStiffness);

		for (int i = 0; i < Particles.Num(); ++i)
			ShapeIndices.Add(i);

		ShapeOffsets.Add(Particles.Num());
	}

	if (ShapeCenters.Num())
	{
		// soft bodies
		Asset->mNumShapes = ShapeCenters.Num();
		Asset->mNumShapeIndices = ShapeIndices.Num();
		Asset->mShapeOffsets = &ShapeOffsets[0];
		Asset->mShapeIndices = &ShapeIndices[0];
		Asset->mShapeCoefficients = &ShapeCoefficients[0];
		Asset->mShapeCenters = (float*)&ShapeCenters[0];
	}

#endif //WITH_FLEX
	return Asset;
}