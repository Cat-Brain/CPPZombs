#include "Include.h"

HMODULE GetModule()
{
	HMODULE hModule;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetModule, &hModule);
	return hModule;
}

class Resource {
public:
	struct Parameters {
		std::size_t size_bytes = 0;
		void* ptr = nullptr;
	};
private:
	HRSRC hResource = nullptr;
	HGLOBAL hMemory = nullptr;

public:
	Parameters p; // Normally private, but it's often nice to be able to access this.

	Resource(int resourceID, int resourceType)
	{
		HMODULE hModule = GetModule();
		HINSTANCE m_hInstance = GetModuleHandle(NULL);
		HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceID), MAKEINTRESOURCE(resourceType));

		if (hResource)
		{
			HGLOBAL hLoadedResource = LoadResource(m_hInstance, hResource);

			if (hLoadedResource)
			{
				LPVOID pLockedResource = LockResource(hLoadedResource);

				if (pLockedResource)
				{
					DWORD dwResourceSize = SizeofResource(m_hInstance, hResource);

					if (0 != dwResourceSize)
					{
						// Use pLockedResource and dwResourceSize however you want
						p.size_bytes = dwResourceSize;
						p.ptr = pLockedResource;
						return;
					}
					else std::cout << "Error with dwResourceSize.\n";
				}
				else std::cout << "Error with pLockedResource.\n";
			}
			else std::cout << "Error with hLoadedResource.\n";
		}
		else
			std::cout << "Error with hResource.\n";
		int lastError = GetLastError();
		std::cout << lastError << "  " << std::system_category().message(lastError);
	}

	auto& GetResource() const {
		return p;
	}

	auto GetResourceString() const {
		std::string_view dst;
		if (p.ptr != nullptr)
			dst = std::string_view(reinterpret_cast<char*>(p.ptr), p.size_bytes);
		return dst;
	}
};