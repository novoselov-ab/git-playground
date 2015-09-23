#include "FlexPCH.h"

// Overrides local vertex factory with CPU skinned deformation
FFlexCPUVertexFactory::FFlexCPUVertexFactory(const FLocalVertexFactory& Base, int NumVerts)
{
	VertexBuffer.Init(NumVerts);

	// have to first initialize our RHI and then recreate it from the static mesh
	BeginInitResource(this);

	// copy vertex factory from LOD0 of staticmesh
	Copy(Base);

	// update position and normal components to point to our vertex buffer
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		InitFlexVertexFactory,
		FFlexCPUVertexFactory*, Factory, this,
		const FFlexVertexBuffer*, VertexBuffer, &VertexBuffer,
		{
			Factory->Data.PositionComponent = FVertexStreamComponent(
			VertexBuffer,
			STRUCT_OFFSET(FFlexVertex, Position),
			sizeof(FFlexVertex),
			VET_Float3
			);

			/*
			Data->TangentBasisComponents[0] = FVertexStreamComponent(
			VertexBuffer,
			STRUCT_OFFSET(FFlexVertex,TangentX),
			sizeof(FFlexVertex),
			VET_Float3
			);
			*/

			Factory->Data.TangentBasisComponents[1] = FVertexStreamComponent(
				VertexBuffer,
				STRUCT_OFFSET(FFlexVertex, TangentZ),
				sizeof(FFlexVertex),
				VET_PackedNormal
				);

			Factory->UpdateRHI();
		});
}

FFlexCPUVertexFactory::~FFlexCPUVertexFactory()
{
	VertexBuffer.ReleaseResource();

	ReleaseResource();
}

void FFlexCPUVertexFactory::SkinCloth(const FVector4* SimulatedPositions, const FVector* SimulatedNormals, const int* VertexToParticleMap)
{
	SCOPE_CYCLE_COUNTER(STAT_Flex_RenderMeshTime);

	FFlexVertex* RESTRICT Vertex = (FFlexVertex*)RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.NumVerts*sizeof(FFlexVertex), RLM_WriteOnly);

	if (SimulatedPositions && SimulatedNormals)
	{
		// update both positions and normals
		for (int i = 0; i < VertexBuffer.NumVerts; ++i)
		{
			const int particleIndex = VertexToParticleMap[i];

			Vertex[i].Position = FVector(SimulatedPositions[particleIndex]);

			// convert normal to packed format
			FPackedNormal Normal = -FVector(SimulatedNormals[particleIndex]);
			Normal.Vector.W = 255;
			Vertex[i].TangentZ = Normal;
		}
	}

	RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
}

void FFlexCPUVertexFactory::SkinSoft(const FPositionVertexBuffer& Positions, const FStaticMeshVertexBuffer& Vertices, const FFlexShapeTransform* Transforms, const FVector* RestPoses, const int16* ClusterIndices, const float* ClusterWeights, int NumClusters)
{
	SCOPE_CYCLE_COUNTER(STAT_Flex_RenderMeshTime);

	const int NumVertices = Vertices.GetNumVertices();

	TArray<FFlexVertex> SkinnedVertices;
	SkinnedVertices.SetNum(NumVertices);

	for (int VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
	{
		FVector SoftPos(0.0f);
		FVector SoftNormal(0.0f);
		FVector SoftTangent(0.0f);

		for (int w = 0; w < 4; ++w)
		{
			const int Cluster = ClusterIndices[VertexIndex * 4 + w];
			const float Weight = ClusterWeights[VertexIndex * 4 + w];

			if (Cluster > -1)
			{
				const FFlexShapeTransform Transform = Transforms[Cluster];

				FVector LocalPos = Positions.VertexPosition(VertexIndex) - RestPoses[Cluster];
				FVector LocalNormal = Vertices.VertexTangentZ(VertexIndex);
				//FVector LocalTangent = Vertices.VertexTangentX(VertexIndex);

				SoftPos += (Transform.Rotation.RotateVector(LocalPos) + Transform.Translation)*Weight;
				SoftNormal += (Transform.Rotation.RotateVector(LocalNormal))*Weight;
				//SoftTangent += Rotation.RotateVector(LocalTangent)*Weight;
			}
		}

		// position
		SkinnedVertices[VertexIndex].Position = SoftPos;

		// normal
		FPackedNormal Normal = SoftNormal;
		Normal.Vector.W = 255;
		SkinnedVertices[VertexIndex].TangentZ = Normal;

		// tangent
		//FPackedNormal Tangent = -SoftTangent;
		//Tangent.Vector.W = 255;
		//SkinnedTangents[VertexIndex] = Tangent;
	}

	FFlexVertex* RESTRICT Vertex = (FFlexVertex*)RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, NumVertices*sizeof(FFlexVertex), RLM_WriteOnly);
	FMemory::Memcpy(Vertex, &SkinnedVertices[0], sizeof(FFlexVertex)*NumVertices);
	RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
}
