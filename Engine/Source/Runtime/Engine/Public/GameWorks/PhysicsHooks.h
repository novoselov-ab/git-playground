#pragma once
#include <functional>
#include "SortedCallbackList.h"

class ENGINE_API FPhysicsHooks
{
public:
	static FPhysicsHooks& get()
	{
		static FPhysicsHooks instance;

		return instance;
	}

	FPhysicsHooks() = default;

	void OnPhysicsSceneEndOfStartFrame(float DeltaSeconds);

	FSortedCallbackList<std::function<void(float DeltaSeconds)>> PhysicsSceneEndOfStartFrameCallbacks;
private:
	FPhysicsHooks(const FPhysicsHooks &) = delete;
	FPhysicsHooks& operator=(const FPhysicsHooks &) = delete;
};

