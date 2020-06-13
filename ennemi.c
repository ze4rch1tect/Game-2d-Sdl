#include <SDL/SDL.h>
#include <math.h>

#include "defs.h"
#include "ennemi.h"
#include "background.h"
#include "hero.h"

int loadEnnemiImages(Ennemi * A)
{
	A->image = IMG_Load("../images/Ennemi.png");

	if(A->image == NULL) {
		printf("Unable to load Ennemi jpg:%s\n", SDL_GetError());
		return (-1);
	}

	

	return (0);
}

  
void initEnnemi(Ennemi* E)
{
	E->positionAbsolue.x = 500;
	E->positionAbsolue.y = 190;
	E->positionAbsolue.w = Ennemi_WIDTH;
	E->positionAbsolue.h = Ennemi_HEIGHT;
	E->positionAnimation.x = 2*Ennemi_WIDTH;
	E->positionAnimation.y = Ennemi_HEIGHT;
        E->positionAnimation2.x = Ennemi_WIDTH*2;
	E->positionAnimation2.y = 0;
	E->positionAnimation.w = Ennemi_WIDTH;
	E->positionAnimation.h = Ennemi_HEIGHT;
        E->xc=E->positionAbsolue.x+72.5;
        E->yc=E->positionAbsolue.y-75;
}
void blitEnnemi(Ennemi E, SDL_Surface* screen)
{
	SDL_BlitSurface(E.image, &E.positionAnimation, screen, &E.positionAbsolue);
}

void MoveEnnemi(Ennemi *E)
{
       E->positionAbsolue.x -=10;    
}
void MoveEnnemi2(Ennemi *E)
{
       E->positionAbsolue.x +=10;    
}

void AnnimateEnnemi(Ennemi *E )
{       
        E->positionAnimation.x += Ennemi_WIDTH;
	E->positionAnimation.x = E->positionAnimation.x % (Ennemi_WIDTH * 3);	
       // E->positionAnimation.y = 0;
        SDL_Delay(100);

}
void AnnimateEnnemi2(Ennemi *E )
{       
        E->positionAnimation2.x +=Ennemi_WIDTH;
	E->positionAnimation2.x = E->positionAnimation.x % (Ennemi_WIDTH * 2);	
        E->positionAnimation2.y = 0;
        SDL_Delay(100);

}
int collision (Ennemi *E,Hero *A)
{
        //int t=((A->xc-E->xc)*(A->xc-E->xc))+((A->yc-E->yc)*(A->yc-E->yc));
        //if (sqrt(t)==(Hero_RADIUS+Ennemi_RADIUS)) 
       if ((E->positionAbsolue.x<=A->positionAbsolue.x+Hero_WIDTH))
        return 1;
        else return 0;
}

	
void freeEnnemi(Ennemi *E)
{
	SDL_FreeSurface(E->image);
}
