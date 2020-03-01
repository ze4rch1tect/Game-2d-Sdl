
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TRANS_R 255
#define TRANS_G 0
#define TRANS_B 255
#define PLAYER_WIDTH 40
#define PLAYER_HEIGTH 80
typedef struct Hero
{
	SDL_Surface *sprite;
	int x, y;
int frameNumber, frameTimer;


} Hero;
SDL_Surface *loadImage(char *name)
{

	SDL_Surface *temp = IMG_Load(name);
	SDL_Surface *image;

	if (temp == NULL)
	{
		printf("Failed to load image %s\n", name);

		return NULL;
	}

	SDL_SetColorKey(temp, (SDL_SRCCOLORKEY|SDL_RLEACCEL), SDL_MapRGB(temp->format, TRANS_R, TRANS_G, TRANS_B));


	image = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	if (image == NULL)
	{
		printf("Failed to convert image %s to native format\n", name);

		return NULL;
	}


	return image;


}
