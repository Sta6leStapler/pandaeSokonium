#include "Game.h"

int main(int argc, char** argv)
{
	Game* game = new Game();
	bool success = game->Initialize();
	if (success)
	{
		game->RunLoop();
	}
	game->Shutdown();

	delete game;

	return 0;
}