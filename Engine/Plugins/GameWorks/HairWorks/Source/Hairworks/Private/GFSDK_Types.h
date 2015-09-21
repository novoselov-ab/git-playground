// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright (c) 2013-2015 NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.
//

#pragma once

#ifndef float4
#define float4			gfsdk_float4
#endif

#ifndef float3
#define float3			gfsdk_float3
#endif

#ifndef float2
#define float2			gfsdk_float2
#endif

#ifndef float4x4
#define float4x4		gfsdk_float4x4
#endif

#ifndef row_major
#define row_major		
#endif

#ifndef float4x4
#define float4x4		gfsdk_float4x4
#endif

#ifndef NOINTERPOLATION
#define	NOINTERPOLATION					
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
struct GFSDK_Hair_ShaderAttributes
{
	float3	P;			// world coord position
	float3	T;			// world space tangent vector
	float3	N;			// world space normal vector at the root
	float4	texcoords; // texture coordinates on hair root 
	// .xy: texcoord on the hair root
	// .z: texcoord along the hair
	// .w: texcoord along the hair quad
	float3	V;			// world space view vector
	float	hairID;		// unique hair identifier

#if defined(GFSDK_HAIR_DECLARE_VELOCITY_ATTR)
	float3	wVel;	// world space velocity
#endif
};

//////////////////////////////////////////////////////////////////////////////
// basic hair material from constant buffer
//////////////////////////////////////////////////////////////////////////////
// 9 float4
struct GFSDK_Hair_Material
{
	// 3 float4
	float4			rootColor;
	float4			tipColor;
	float4			specularColor;

	// 4 floats (= 1 float4)
	float			diffuseBlend;
	float			diffuseScale;
	float			diffuseHairNormalWeight;
	float			_diffuseUnused_; // for alignment and future use

	// 4 floats (= 1 float4)
	float			specularPrimaryScale;
	float			specularPrimaryPower;
	float			specularPrimaryBreakup;
	float			specularNoiseScale;

	// 4 floats (= 1 float4)
	float			specularSecondaryScale;
	float			specularSecondaryPower;
	float			specularSecondaryOffset;
	float			_specularUnused_; // for alignment and future use

	// 4 floats (= 1 float4)
	float			rootTipColorWeight;
	float			rootTipColorFalloff;
	float			shadowSigma;
	float			strandBlendScale;

	// 4 floats (= 1 float4)
	float			glintStrength;
	float			glintCount;
	float			glintExponent;
	float			rootAlphaFalloff;
};

//////////////////////////////////////////////////////////////////////////////
// Use this data structure to manage all hair related cbuffer data within your own cbuffer
struct GFSDK_Hair_ConstantBuffer
{
	// camera information 
	row_major	float4x4	inverseViewProjection; // inverse of view projection matrix
	row_major	float4x4	inverseProjection; // inverse of projection matrix
	row_major	float4x4	inverseViewport; // inverse of viewport transform
	row_major	float4x4	inverseViewProjectionViewport; // inverse of world to screen matrix

	row_major	float4x4	viewProjection; // view projection matrix
	row_major	float4x4	viewport; // viewport matrix

	row_major	float4x4	prevViewProjection; // previous view projection matrix for pixel velocity computation
	row_major	float4x4	prevViewport; // previous viewport matrix for pixel velocity computation

	float4					camPosition;		  // position of camera center
	float4					modelCenter; // center of the growth mesh model

	// shared settings 
	int						useRootColorTexture;
	int						useTipColorTexture;
	int						useStrandTexture;
	int						useSpecularTexture;

	int						receiveShadows;
	int						shadowUseLeftHanded;
	float					__shadowReserved1__;
	float					__shadowReserved2__;

	int						strandBlendMode;
	int						colorizeMode;
	int						strandPointCount;
	int						__reserved__;

	float					lodDistanceFactor;
	float					lodDetailFactor;
	float					lodAlphaFactor;
	float					__reservedLOD___;


	GFSDK_Hair_Material		defaultMaterial;

	// noise table
	float4					_noiseTable[128]; // 1024 floats	// 256 is too large, use 128
};