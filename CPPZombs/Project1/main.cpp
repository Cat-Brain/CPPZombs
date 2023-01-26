//#include "Game2.h"
#include "Renderer.h"

int main()
{
	printf("Greetings universe!\nDo you want to use fullscreen? 'y' or 'n'");

	//for (int i = 0; i < Plants::plants.size(); i++)
		//Plants::plants[i]->seed = Collectibles::Seeds::plantSeeds[i];

#pragma region Icons

	/*HRSRC hResource = FindResource(m_hInstance, MAKEINTRESOURCE(""), L"TEXT");

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
				}
			}
		}
	}*/

#pragma endregion

	string fullscreen;
	std::cin >> fullscreen;
	while (fullscreen != "y" && fullscreen != "n")
		std::cin >> fullscreen;

	Renderer renderer;
	renderer.Construct();
	//game = make_unique<Game>();
	//if (game->Construct(screenWidthHighRes, screenHeightHighRes, 2, 2, fullscreen == "y", true)) game->Start();

	printf("\nFairwell universe!\n");
	return 0;
}