#pragma once

// if true GPU skinning will be used for soft bodies on SM4+ devices
#define USE_FLEX_GPU_SKINNING 1	

struct FFlexShapeTransform
{
	FVector Translation;
	FQuat Rotation;
};


