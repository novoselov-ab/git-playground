#pragma once

#if STATS

DECLARE_STATS_GROUP(TEXT("Flex"), STATGROUP_Flex, STATCAT_Advanced);
DECLARE_STATS_GROUP_VERBOSE(TEXT("FlexGpu"), STATGROUP_FlexGpu, STATCAT_Advanced);

enum EFlexStats
{
	// UFlexComponentStats stats
	STAT_Flex_RenderMeshTime,
	STAT_Flex_UpdateBoundsCpu,
	STAT_Flex_ActiveParticleCount,
	STAT_Flex_ActiveMeshActorCount,

	// Container stats
	STAT_Flex_DeviceUpdateTime,
	STAT_Flex_SolverUpdateTime,
	STAT_Flex_WaitTime,
	STAT_Flex_GatherCollisionShapes,
	STAT_Flex_UpdateCollisionShapes,
	STAT_Flex_UpdateActors,
	STAT_Flex_ContainerCount,
	STAT_Flex_InstanceCount,
	STAT_Flex_ParticleCount,
	STAT_Flex_SpringCount,
	STAT_Flex_ShapeCount,
	STAT_Flex_StaticConvexCount,
	STAT_Flex_StaticTriangleCount,
	STAT_Flex_ForceFieldCount,
};

enum EFlexGpuStats
{
	// gpu stats
	STAT_Flex_ContainerGpuTickTime,
	STAT_Flex_Predict,
	STAT_Flex_CreateCellIndices,
	STAT_Flex_SortCellIndices,
	STAT_Flex_CreateGrid,
	STAT_Flex_Reorder,
	STAT_Flex_CollideParticles,
	STAT_Flex_CollideConvexes,
	STAT_Flex_CollideTriangles,
	STAT_Flex_CollideFields,
	STAT_Flex_CalculateDensity,
	STAT_Flex_SolveDensities,
	STAT_Flex_SolveVelocities,
	STAT_Flex_SolveShapes,
	STAT_Flex_SolveSprings,
	STAT_Flex_SolveContacts,
	STAT_Flex_SolveInflatables,
	STAT_Flex_CalculateAnisotropy,
	STAT_Flex_UpdateDiffuse,
	STAT_Flex_UpdateTriangles,
	STAT_Flex_Finalize,
	STAT_Flex_UpdateBounds,
};


#endif