#include "FlexPCH.h"

void FFlexVertexFactory::AddVertexPositionElements(DataType& Data, FVertexDeclarationElementList& Elements)
{
	Elements.Add(AccessPositionStreamComponent(Data.PositionComponent, 0));
}

void FFlexVertexFactory::AddVertexElements(DataType& Data, FVertexDeclarationElementList& Elements)
{
	if (Data.PositionComponent.VertexBuffer != NULL)
	{
		Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
	}

	// only tangent,normal are used by the stream. the binormal is derived in the shader
	uint8 TangentBasisAttributes[2] = { 1, 2 };
	for (int32 AxisIndex = 0; AxisIndex < 2; AxisIndex++)
	{
		if (Data.TangentBasisComponents[AxisIndex].VertexBuffer != NULL)
		{
			Elements.Add(AccessStreamComponent(Data.TangentBasisComponents[AxisIndex], TangentBasisAttributes[AxisIndex]));
		}
	}

	if (Data.ColorComponent.VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.ColorComponent, 3));
	}
	else
	{
		//If the mesh has no color component, set the null color buffer on a new stream with a stride of 0.
		//This wastes 4 bytes of bandwidth per vertex, but prevents having to compile out twice the number of vertex factories.
		FVertexStreamComponent NullColorComponent(&GNullColorVertexBuffer, 0, 0, VET_Color);
		Elements.Add(AccessStreamComponent(NullColorComponent, 3));
	}

	if (Data.TextureCoordinates.Num())
	{
		const int32 BaseTexCoordAttribute = 4;
		for (int32 CoordinateIndex = 0; CoordinateIndex < Data.TextureCoordinates.Num(); CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(
				Data.TextureCoordinates[CoordinateIndex],
				BaseTexCoordAttribute + CoordinateIndex
				));
		}

		for (int32 CoordinateIndex = Data.TextureCoordinates.Num(); CoordinateIndex < MAX_STATIC_TEXCOORDS / 2; CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(
				Data.TextureCoordinates[Data.TextureCoordinates.Num() - 1],
				BaseTexCoordAttribute + CoordinateIndex
				));
		}
	}

	if (Data.LightMapCoordinateComponent.VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.LightMapCoordinateComponent, 15));
	}
	else if (Data.TextureCoordinates.Num())
	{
		Elements.Add(AccessStreamComponent(Data.TextureCoordinates[0], 15));
	}

}