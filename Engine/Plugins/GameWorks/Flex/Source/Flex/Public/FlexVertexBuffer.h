#pragma once
#include "RenderResource.h"

class FFlexVertexBuffer : public FVertexBuffer
{
public:

	int NumVerts;

	void Init(int Count);

	virtual ~FFlexVertexBuffer() {}
	virtual void InitRHI() override;
};
