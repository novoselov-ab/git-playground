/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// This file was generated by NxParameterized/scripts/GenParameterized.pl
// Created: 2015.06.02 04:12:11

#ifndef HEADER_EmitterGeomExplicitParams_0p1_h
#define HEADER_EmitterGeomExplicitParams_0p1_h

#include "NxParametersTypes.h"

#ifndef NX_PARAMETERIZED_ONLY_LAYOUTS
#include "NxParameterized.h"
#include "NxParameters.h"
#include "NxParameterizedTraits.h"
#include "NxTraitsInternal.h"
#endif

namespace physx
{
namespace apex
{

#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to __declspec(align())

namespace EmitterGeomExplicitParams_0p1NS
{

struct PointParams_Type;
struct ExplicitPoints_Type;
struct SphereParams_Type;
struct ExplicitSpheres_Type;
struct EllipsoidParams_Type;
struct ExplicitEllipsoids_Type;

struct PointParams_DynamicArray1D_Type
{
	PointParams_Type* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct VEC3_DynamicArray1D_Type
{
	physx::PxVec3* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct SphereParams_DynamicArray1D_Type
{
	SphereParams_Type* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct EllipsoidParams_DynamicArray1D_Type
{
	EllipsoidParams_Type* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct PointParams_Type
{
	physx::PxVec3 position;
	bool doDetectOverlaps;
};
struct EllipsoidParams_Type
{
	physx::PxVec3 center;
	physx::PxF32 radius;
	physx::PxVec3 normal;
	physx::PxF32 polarRadius;
	bool doDetectOverlaps;
};
struct ExplicitEllipsoids_Type
{
	EllipsoidParams_DynamicArray1D_Type positions;
	VEC3_DynamicArray1D_Type velocities;
};
struct ExplicitPoints_Type
{
	PointParams_DynamicArray1D_Type positions;
	VEC3_DynamicArray1D_Type velocities;
};
struct SphereParams_Type
{
	physx::PxVec3 center;
	physx::PxF32 radius;
	bool doDetectOverlaps;
};
struct ExplicitSpheres_Type
{
	SphereParams_DynamicArray1D_Type positions;
	VEC3_DynamicArray1D_Type velocities;
};

struct ParametersStruct
{

	physx::PxF32 distance;
	ExplicitPoints_Type points;
	ExplicitSpheres_Type spheres;
	ExplicitEllipsoids_Type ellipsoids;

};

static const physx::PxU32 checksum[] = { 0xe09894f3, 0xce4efef3, 0x4e950875, 0x6ea40ff1, };

} // namespace EmitterGeomExplicitParams_0p1NS

#ifndef NX_PARAMETERIZED_ONLY_LAYOUTS
class EmitterGeomExplicitParams_0p1 : public NxParameterized::NxParameters, public EmitterGeomExplicitParams_0p1NS::ParametersStruct
{
public:
	EmitterGeomExplicitParams_0p1(NxParameterized::Traits* traits, void* buf = 0, PxI32* refCount = 0);

	virtual ~EmitterGeomExplicitParams_0p1();

	virtual void destroy();

	static const char* staticClassName(void)
	{
		return("EmitterGeomExplicitParams");
	}

	const char* className(void) const
	{
		return(staticClassName());
	}

	static const physx::PxU32 ClassVersion = ((physx::PxU32)0 << 16) + (physx::PxU32)1;

	static physx::PxU32 staticVersion(void)
	{
		return ClassVersion;
	}

	physx::PxU32 version(void) const
	{
		return(staticVersion());
	}

	static const physx::PxU32 ClassAlignment = 8;

	static const physx::PxU32* staticChecksum(physx::PxU32& bits)
	{
		bits = 8 * sizeof(EmitterGeomExplicitParams_0p1NS::checksum);
		return EmitterGeomExplicitParams_0p1NS::checksum;
	}

	static void freeParameterDefinitionTable(NxParameterized::Traits* traits);

	const physx::PxU32* checksum(physx::PxU32& bits) const
	{
		return staticChecksum(bits);
	}

	const EmitterGeomExplicitParams_0p1NS::ParametersStruct& parameters(void) const
	{
		EmitterGeomExplicitParams_0p1* tmpThis = const_cast<EmitterGeomExplicitParams_0p1*>(this);
		return *(static_cast<EmitterGeomExplicitParams_0p1NS::ParametersStruct*>(tmpThis));
	}

	EmitterGeomExplicitParams_0p1NS::ParametersStruct& parameters(void)
	{
		return *(static_cast<EmitterGeomExplicitParams_0p1NS::ParametersStruct*>(this));
	}

	virtual NxParameterized::ErrorType getParameterHandle(const char* long_name, NxParameterized::Handle& handle) const;
	virtual NxParameterized::ErrorType getParameterHandle(const char* long_name, NxParameterized::Handle& handle);

	void initDefaults(void);

protected:

	virtual const NxParameterized::DefinitionImpl* getParameterDefinitionTree(void);
	virtual const NxParameterized::DefinitionImpl* getParameterDefinitionTree(void) const;


	virtual void getVarPtr(const NxParameterized::Handle& handle, void*& ptr, size_t& offset) const;

private:

	void buildTree(void);
	void initDynamicArrays(void);
	void initStrings(void);
	void initReferences(void);
	void freeDynamicArrays(void);
	void freeStrings(void);
	void freeReferences(void);

	static bool mBuiltFlag;
	static NxParameterized::MutexType mBuiltFlagMutex;
};

class EmitterGeomExplicitParams_0p1Factory : public NxParameterized::Factory
{
	static const char* const vptr;

public:
	virtual NxParameterized::Interface* create(NxParameterized::Traits* paramTraits)
	{
		// placement new on this class using mParameterizedTraits

		void* newPtr = paramTraits->alloc(sizeof(EmitterGeomExplicitParams_0p1), EmitterGeomExplicitParams_0p1::ClassAlignment);
		if (!NxParameterized::IsAligned(newPtr, EmitterGeomExplicitParams_0p1::ClassAlignment))
		{
			NX_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class EmitterGeomExplicitParams_0p1");
			paramTraits->free(newPtr);
			return 0;
		}

		memset(newPtr, 0, sizeof(EmitterGeomExplicitParams_0p1)); // always initialize memory allocated to zero for default values
		return NX_PARAM_PLACEMENT_NEW(newPtr, EmitterGeomExplicitParams_0p1)(paramTraits);
	}

	virtual NxParameterized::Interface* finish(NxParameterized::Traits* paramTraits, void* bufObj, void* bufStart, physx::PxI32* refCount)
	{
		if (!NxParameterized::IsAligned(bufObj, EmitterGeomExplicitParams_0p1::ClassAlignment)
		        || !NxParameterized::IsAligned(bufStart, EmitterGeomExplicitParams_0p1::ClassAlignment))
		{
			NX_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class EmitterGeomExplicitParams_0p1");
			return 0;
		}

		// Init NxParameters-part
		// We used to call empty constructor of EmitterGeomExplicitParams_0p1 here
		// but it may call default constructors of members and spoil the data
		NX_PARAM_PLACEMENT_NEW(bufObj, NxParameterized::NxParameters)(paramTraits, bufStart, refCount);

		// Init vtable (everything else is already initialized)
		*(const char**)bufObj = vptr;

		return (EmitterGeomExplicitParams_0p1*)bufObj;
	}

	virtual const char* getClassName()
	{
		return (EmitterGeomExplicitParams_0p1::staticClassName());
	}

	virtual physx::PxU32 getVersion()
	{
		return (EmitterGeomExplicitParams_0p1::staticVersion());
	}

	virtual physx::PxU32 getAlignment()
	{
		return (EmitterGeomExplicitParams_0p1::ClassAlignment);
	}

	virtual const physx::PxU32* getChecksum(physx::PxU32& bits)
	{
		return (EmitterGeomExplicitParams_0p1::staticChecksum(bits));
	}
};
#endif // NX_PARAMETERIZED_ONLY_LAYOUTS

} // namespace apex
} // namespace physx

#pragma warning(pop)

#endif
