#include "EnginePrivate.h"

void FPhysicsHooks::OnPhysicsSceneEndOfStartFrame(float DeltaSeconds)
{
	for (auto callback : PhysicsSceneEndOfStartFrameCallbacks.getCallbacks())
	{
		callback->getCallback()(DeltaSeconds);
	}
}
