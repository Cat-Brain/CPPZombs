#include "Include.h"

HMODULE GCM()
{
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GCM,
		&hModule
	);
	return hModule;
}

class Resource {
public:
	std::size_t size = 0;
	void* ptr = nullptr;

	Resource(int resourceID, int resourceType)
	{
		HMODULE hModule = GCM();
		HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceID), MAKEINTRESOURCE(resourceType));
		HGLOBAL hData = LoadResource(hModule, hResource);
		size = SizeofResource(hModule, hResource);
		ptr = LockResource(hData);
	}
};