
#include "player.h"


void initializePlayer(void)
{

    player.sprite = loadImage("graphics/walkright.png");

    player.x = 20;
    player.y = 304;

}


void drawplayer()
{
	SDL_Rect dest;

	dest.x = player.x;
	dest.y = player.y;
	dest.w = PLAYER_WIDTH;
	dest.h = PLAYER_HEIGTH;

	SDL_Rect src;

	src.x = 0;
	src.y = 0;
	src.w = PLAYER_WIDTH;
	src.h = PLAYER_HEIGTH;

	SDL_BlitSurface(player.sprite, &src, jeu.screen, &dest);

}

