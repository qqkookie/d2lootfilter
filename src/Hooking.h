#pragma once

namespace Hooking {
	bool Hook(void* src, void* dst, int len);
	void TrampolineHook(void* src, void* dst, void** orig, int len);
	void SetCall(void* address, void* function, size_t size);
	void SetJmp(void* address, void* function, size_t size);
};
