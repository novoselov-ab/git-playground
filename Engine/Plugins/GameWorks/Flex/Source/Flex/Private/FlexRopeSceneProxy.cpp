#include "FlexPCH.h"

FFlexRopeSceneProxy::FFlexRopeSceneProxy(UFlexRopeComponent* Component) : FPrimitiveSceneProxy(Component)
, Material(NULL)
, DynamicData(NULL)
, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
, NumSegments(Component->NumSegments)
, Width(Component->Width)
, NumSides(Component->NumSides)
, TileMaterial(Component->TileMaterial)
{
	VertexBuffer.NumVerts = GetRequiredVertexCount();
	IndexBuffer.NumIndices = GetRequiredIndexCount();

	// Init vertex factory
	VertexFactory.Init(&VertexBuffer);

	// Enqueue initialization of render resource
	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	// Grab material
	Material = Component->GetMaterial(0);
	if (Material == NULL)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

FFlexRopeSceneProxy::~FFlexRopeSceneProxy()
{
	VertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();

	if (DynamicData != NULL)
	{
		delete DynamicData;
	}
}

int32 FFlexRopeSceneProxy::GetRequiredVertexCount() const
{
	return (NumSegments + 1) * (NumSides + 1);
}

int32 FFlexRopeSceneProxy::GetRequiredIndexCount() const
{
	return (NumSegments * NumSides * 2) * 3;
}

int32 FFlexRopeSceneProxy::GetVertIndex(int32 AlongIdx, int32 AroundIdx) const
{
	return (AlongIdx * (NumSides + 1)) + AroundIdx;
}

void FFlexRopeSceneProxy::BuildRopeMesh(const TArray<FVector>& InPoints, TArray<FDynamicMeshVertex>& OutVertices, TArray<int32>& OutIndices)
{
	const FColor VertexColor(255, 255, 255);
	const int32 NumPoints = InPoints.Num();
	const int32 SegmentCount = NumPoints - 1;

	// Build vertices

	// We double up the first and last vert of the ring, because the UVs are different
	int32 NumRingVerts = NumSides + 1;

	if (NumPoints < 2)
		return;

	FVector BasisX = (InPoints[1] - InPoints[0]).GetSafeNormal();
	FVector BasisY, BasisZ;
	BasisX.FindBestAxisVectors(BasisY, BasisZ);

	// For each point along spline..
	for (int32 PointIdx = 0; PointIdx<NumPoints; PointIdx++)
	{
		const float AlongFrac = (float)PointIdx / (float)SegmentCount; // Distance along FlexRope

		// Find direction of FlexRope at this point, by averaging previous and next points
		const int32 PrevIndex = FMath::Max(0, PointIdx - 1);
		const int32 NextIndex = FMath::Min(PointIdx + 1, NumPoints - 1);
		const FVector ForwardDir = (InPoints[NextIndex] - InPoints[PrevIndex]).GetSafeNormal();

		FVector RotationAxis = (BasisX ^ ForwardDir).GetSafeNormal();
		float CosTheta = ForwardDir | BasisX;

		// Use a parallel transport frame to create a smooth basis along the rope
		if (FMath::Abs(CosTheta - 1.0f) > KINDA_SMALL_NUMBER)
		{
			BasisX = ForwardDir;

			// TODO: trigonometric functions totally unnecessary here
			float Theta = FMath::RadiansToDegrees(FMath::Acos(CosTheta));

			BasisY = BasisY.RotateAngleAxis(Theta, RotationAxis);
			BasisZ = BasisZ.RotateAngleAxis(Theta, RotationAxis);
		}

		// Generate a ring of verts
		for (int32 VertIdx = 0; VertIdx < NumRingVerts; VertIdx++)
		{
			const float AroundFrac = float(VertIdx) / float(NumSides);
			// Find angle around the ring
			const float RadAngle = 2.f * PI * AroundFrac;
			// Find direction from center of FlexRope to this vertex
			const FVector OutDir = (FMath::Cos(RadAngle) * BasisY) + (FMath::Sin(RadAngle) * BasisZ);

			FDynamicMeshVertex Vert;
			Vert.Position = InPoints[PointIdx] + (OutDir * 0.5f * Width);
			Vert.TextureCoordinate = FVector2D(AlongFrac * TileMaterial, AroundFrac);
			Vert.Color = VertexColor;
			Vert.SetTangents(ForwardDir, OutDir ^ ForwardDir, OutDir);
			OutVertices.Add(Vert);
		}
	}

	// Build triangles
	for (int32 SegIdx = 0; SegIdx < SegmentCount; SegIdx++)
	{
		for (int32 SideIdx = 0; SideIdx < NumSides; SideIdx++)
		{
			int32 TL = GetVertIndex(SegIdx, SideIdx);
			int32 BL = GetVertIndex(SegIdx, SideIdx + 1);
			int32 TR = GetVertIndex(SegIdx + 1, SideIdx);
			int32 BR = GetVertIndex(SegIdx + 1, SideIdx + 1);

			OutIndices.Add(TL);
			OutIndices.Add(BL);
			OutIndices.Add(TR);

			OutIndices.Add(TR);
			OutIndices.Add(BL);
			OutIndices.Add(BR);
		}
	}
}

void FFlexRopeSceneProxy::SetDynamicData_RenderThread(FFlexRopeDynamicData* NewDynamicData)
{
	check(IsInRenderingThread());

	// Free existing data if present
	if (DynamicData)
	{
		delete DynamicData;
		DynamicData = NULL;
	}
	DynamicData = NewDynamicData;

	// Build mesh from FlexRope points
	TArray<FDynamicMeshVertex> Vertices;
	TArray<int32> Indices;
	BuildRopeMesh(NewDynamicData->FlexRopePoints, Vertices, Indices);

	check(Vertices.Num() == GetRequiredVertexCount());
	check(Indices.Num() == GetRequiredIndexCount());

	void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
	FMemory::Memcpy(VertexBufferData, &Vertices[0], Vertices.Num() * sizeof(FDynamicMeshVertex));
	RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);

	void* IndexBufferData = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
	FMemory::Memcpy(IndexBufferData, &Indices[0], Indices.Num() * sizeof(int32));
	RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
}

void FFlexRopeSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FlexRopeSceneProxy_GetDynamicMeshElements);

	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
		GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
		FLinearColor(0, 0.5f, 1.f)
		);

	Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

	FMaterialRenderProxy* MaterialProxy = NULL;
	if (bWireframe)
	{
		MaterialProxy = WireframeMaterialInstance;
	}
	else
	{
		MaterialProxy = Material->GetRenderProxy(IsSelected());
	}

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];
			// Draw the mesh.
			FMeshBatch& Mesh = Collector.AllocateMesh();
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &IndexBuffer;
			Mesh.bWireframe = bWireframe;
			Mesh.VertexFactory = &VertexFactory;
			Mesh.MaterialRenderProxy = MaterialProxy;
			BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = GetRequiredIndexCount() / 3;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = GetRequiredVertexCount();
			Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			Mesh.Type = PT_TriangleList;
			Mesh.DepthPriorityGroup = SDPG_World;
			Mesh.bCanApplyViewModeOverrides = false;
			Collector.AddMesh(ViewIndex, Mesh);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			// Render bounds
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
#endif
		}
	}
}

void FFlexRopeSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FlexRopeSceneProxy_DrawDynamicElements);

	const bool bWireframe = AllowDebugViewmodes() && View->Family->EngineShowFlags.Wireframe;

	FColoredMaterialRenderProxy WireframeMaterialInstance(
		GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
		FLinearColor(0, 0.5f, 1.f)
		);

	FMaterialRenderProxy* MaterialProxy = NULL;
	if (bWireframe)
	{
		MaterialProxy = &WireframeMaterialInstance;
	}
	else
	{
		MaterialProxy = Material->GetRenderProxy(IsSelected());
	}

	// Draw the mesh.
	FMeshBatch Mesh;
	FMeshBatchElement& BatchElement = Mesh.Elements[0];
	BatchElement.IndexBuffer = &IndexBuffer;
	Mesh.bWireframe = bWireframe;
	Mesh.VertexFactory = &VertexFactory;
	Mesh.MaterialRenderProxy = MaterialProxy;
	BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = GetRequiredIndexCount() / 3;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = GetRequiredVertexCount();
	Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
	Mesh.Type = PT_TriangleList;
	Mesh.DepthPriorityGroup = SDPG_World;
	PDI->DrawMesh(Mesh);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// Render bounds
	RenderBounds(PDI, View->Family->EngineShowFlags, GetBounds(), IsSelected());
#endif
}

FPrimitiveViewRelevance FFlexRopeSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	return Result;
}

uint32 FFlexRopeSceneProxy::GetMemoryFootprint(void) const
{
	return(sizeof(*this) + GetAllocatedSize());
}

uint32 FFlexRopeSceneProxy::GetAllocatedSize(void) const
{
	return(FPrimitiveSceneProxy::GetAllocatedSize());
}

void FFlexRopeVertexFactory::Init(const FFlexRopeVertexBuffer* VertexBuffer)
{
	if (IsInRenderingThread())
	{
		// Initialize the vertex factory's stream components.
		DataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
		NewData.TextureCoordinates.Add(
			FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate), sizeof(FDynamicMeshVertex), VET_Float2)
			);
		NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
		SetData(NewData);
	}
	else
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			InitFlexRopeVertexFactory,
			FFlexRopeVertexFactory*, VertexFactory, this,
			const FFlexRopeVertexBuffer*, VertexBuffer, VertexBuffer,
			{
				// Initialize the vertex factory's stream components.
				DataType NewData;
				NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
				NewData.TextureCoordinates.Add(
					FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate), sizeof(FDynamicMeshVertex), VET_Float2)
					);
				NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
				NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
				VertexFactory->SetData(NewData);
			});
	}
}

void FFlexRopeVertexBuffer::InitRHI()
{
	FRHIResourceCreateInfo CreateInfo;
	VertexBufferRHI = RHICreateVertexBuffer(NumVerts * sizeof(FDynamicMeshVertex), BUF_Dynamic, CreateInfo);
}

void FFlexRopeIndexBuffer::InitRHI()
{
	FRHIResourceCreateInfo CreateInfo;
	IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), NumIndices * sizeof(int32), BUF_Dynamic, CreateInfo);
}
