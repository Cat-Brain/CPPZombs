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
		if (hResource != 0)
		{
			HGLOBAL hData = LoadResource(hModule, hResource);
			if (hData != 0)
			{
				size = SizeofResource(hModule, hResource);
				if (size != 0)
					ptr = LockResource(hData);
			}
		}
	}
};