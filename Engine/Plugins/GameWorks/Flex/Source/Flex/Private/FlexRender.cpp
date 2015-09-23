#include "FlexPCH.h"

#if STATS
DECLARE_CYCLE_STAT(TEXT("Skin Mesh Time (CPU)"), STAT_Flex_RenderMeshTime, STATGROUP_Flex);
#endif


/* CPU Skinning */






/* GPU Skinning */

class FFlexMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	virtual void Bind(const FShaderParameterMap& ParameterMap) override;
	virtual void SetMesh(FRHICommandList& RHICmdList, FShader* Shader, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, uint32 DataFlags) const override;
	void Serialize(FArchive& Ar) override;
	virtual uint32 GetSize() const override;

private:

	FShaderResourceParameter ClusterTranslationsParameter;
	FShaderResourceParameter ClusterRotationsParameter;
};




