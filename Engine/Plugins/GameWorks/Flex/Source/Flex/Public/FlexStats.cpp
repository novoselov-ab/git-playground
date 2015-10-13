#include "FlexPCH.h"

DECLARE_CYCLE_STAT(TEXT("Skin Mesh Time (CPU)"), STAT_Flex_RenderMeshTime, STATGROUP_Flex);

DECLARE_CYCLE_STAT(TEXT("Update Bounds (CPU)"), STAT_Flex_UpdateBoundsCpu, STATGROUP_Flex);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Active Mesh Particle Count"), STAT_Flex_ActiveParticleCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Active Mesh Actor Count"), STAT_Flex_ActiveMeshActorCount, STATGROUP_Flex);

// CPU stats, use "stat flex" to enable
DECLARE_CYCLE_STAT(TEXT("Gather Collision Shapes Time (CPU)"), STAT_Flex_GatherCollisionShapes, STATGROUP_Flex);
DECLARE_CYCLE_STAT(TEXT("Update Collision Shapes Time (CPU)"), STAT_Flex_UpdateCollisionShapes, STATGROUP_Flex);
DECLARE_CYCLE_STAT(TEXT("Update Actors Time (CPU)"), STAT_Flex_UpdateActors, STATGROUP_Flex);
DECLARE_CYCLE_STAT(TEXT("Update Data Time (CPU)"), STAT_Flex_DeviceUpdateTime, STATGROUP_Flex);
DECLARE_CYCLE_STAT(TEXT("Solver Tick Time (CPU)"), STAT_Flex_SolverUpdateTime, STATGROUP_Flex);

// Counters
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Container Count"), STAT_Flex_ContainerCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Instance Count"), STAT_Flex_InstanceCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Particle Count"), STAT_Flex_ParticleCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Spring Count"), STAT_Flex_SpringCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Shape Count"), STAT_Flex_ShapeCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Static Convex Count"), STAT_Flex_StaticConvexCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Static Triangle Count"), STAT_Flex_StaticTriangleCount, STATGROUP_Flex);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Force Field Count"), STAT_Flex_ForceFieldCount, STATGROUP_Flex);

// GPU stats, use "stat group enable flexgpu", and "stat flexgpu" to enable via console
// note that the the GPU counters will introduce significant synchronization overhead
DECLARE_CYCLE_STAT(TEXT("Predict"), STAT_Flex_Predict, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("CreateCellIndices"), STAT_Flex_CreateCellIndices, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("SortCellIndices"), STAT_Flex_SortCellIndices, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("CreateGrid"), STAT_Flex_CreateGrid, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Reorder"), STAT_Flex_Reorder, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Collide Particles"), STAT_Flex_CollideParticles, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Collide Convexes"), STAT_Flex_CollideConvexes, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Collide Triangles"), STAT_Flex_CollideTriangles, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Collide Fields"), STAT_Flex_CollideFields, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Calculate Density"), STAT_Flex_CalculateDensity, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Solve Density"), STAT_Flex_SolveDensities, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Solve Velocities"), STAT_Flex_SolveVelocities, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Solve Shapes"), STAT_Flex_SolveShapes, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Solve Springs"), STAT_Flex_SolveSprings, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Solve Contacts"), STAT_Flex_SolveContacts, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Solve Inflatables"), STAT_Flex_SolveInflatables, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Calculate Anisotropy"), STAT_Flex_CalculateAnisotropy, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Update Diffuse"), STAT_Flex_UpdateDiffuse, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Finalize"), STAT_Flex_Finalize, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Update Bounds"), STAT_Flex_UpdateBounds, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Update Triangles"), STAT_Flex_UpdateTriangles, STATGROUP_FlexGpu);
DECLARE_CYCLE_STAT(TEXT("Total GPU Kernel Time"), STAT_Flex_ContainerGpuTickTime, STATGROUP_FlexGpu);
