#include "FlexPCH.h"

// factory shader parameter implementation methods

void FFlexMeshVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	ClusterTranslationsParameter.Bind(ParameterMap, TEXT("ClusterTranslations"), SPF_Optional);
	ClusterRotationsParameter.Bind(ParameterMap, TEXT("ClusterRotations"), SPF_Optional);
}

void FFlexMeshVertexFactoryShaderParameters::SetMesh(FRHICommandList& RHICmdList, FShader* Shader, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, uint32 DataFlags) const
{
	if (Shader->GetVertexShader())
	{
		FFlexGPUVertexFactory* Factory = (FFlexGPUVertexFactory*)(VertexFactory);

		if (ClusterTranslationsParameter.IsBound())
		{
			RHICmdList.SetShaderResourceViewParameter(Shader->GetVertexShader(), ClusterTranslationsParameter.GetBaseIndex(), Factory->ClusterTranslations.SRV);
		}

		if (ClusterRotationsParameter.IsBound())
		{
			RHICmdList.SetShaderResourceViewParameter(Shader->GetVertexShader(), ClusterRotationsParameter.GetBaseIndex(), Factory->ClusterRotations.SRV);
		}
	}
}

void FFlexMeshVertexFactoryShaderParameters::Serialize(FArchive& Ar)
{
	Ar << ClusterTranslationsParameter;
	Ar << ClusterRotationsParameter;
}

uint32 FFlexMeshVertexFactoryShaderParameters::GetSize() const
{
	return sizeof(*this);
}