

#include "main.h"


int main(int argc, char *argv[])
{
	unsigned int frameLimit = SDL_GetTicks() + 16;
	int go;
	initializePlayer();

	go = 1;


	while (go == 1)
	{
		drawplayer();
		delay(frameLimit);
		frameLimit = SDL_GetTicks() + 16;

	}

	exit(0);
}

