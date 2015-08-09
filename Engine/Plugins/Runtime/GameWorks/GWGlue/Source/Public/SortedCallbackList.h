#pragma once
#include <vector>
#include <algorithm>
#include <memory>


template <typename TCallback>
class FSortableCallback
{
public:

	FSortableCallback(TCallback callback, int8 priority, uint8 handle) { Callback = callback; priority = priority; Handle = handle; }

	inline TCallback& getCallback() { return Callback; }
	inline int8 getPriority() { return Priority; }
	inline uint8 getHandle() { return Handle; }

	FSortableCallback(const FSortableCallback&) = delete;
	FSortableCallback& operator=(const FSortableCallback&) = delete;

private:
	TCallback	Callback;
	int8		Priority;
	uint8		Handle;
};


template <typename TCallback>
class FSortedCallbackList
{
public:

	uint8 Add(TCallback callback, int8 priority);
	void Remove(uint8 callbackHandle);

	inline std::vector<std::shared_ptr<FSortableCallback<TCallback>>>& getCallbacks() { return Callbacks; }

private:

	std::vector<std::shared_ptr<FSortableCallback<TCallback>>> Callbacks;
};

template <typename TCallback>
uint8 FSortedCallbackList<TCallback>::Add(TCallback callback, int8 priority)
{
	auto callback = std::make_shared<FSortableCallback<TCallback>>(callback, priority, Callbacks.size());
	Callbacks.push_back(callback);

	std::sort(Callbacks.begin(), Callbacks.end(), [](const FSortableCallback<TCallback>& lhs, const FSortableCallback<TCallback>& rhs)
	{
		return lhs.getPriority() < rhs.getPriority();
	});

	return callback->getHandle();
}

template <typename TCallback>
void FSortedCallbackList<TCallback>::Remove(uint8 callbackHandle)
{
	auto position = std::find_if(Callbacks.begin(), Callbacks.end(), [&callbackHandle](const FSortableCallback<TCallback> &arg) { return arg.getHandle() == callbackHandle; });
}

