#include "FlexPCH.h"

void FFlexVertexBuffer::Init(int Count)
{
	NumVerts = Count;

	BeginInitResource(this);
}

void FFlexVertexBuffer::InitRHI()
{
	// Create the vertex buffer.
	FRHIResourceCreateInfo CreateInfo;
	VertexBufferRHI = RHICreateVertexBuffer(NumVerts*sizeof(FFlexVertex), BUF_AnyDynamic, CreateInfo);
}
