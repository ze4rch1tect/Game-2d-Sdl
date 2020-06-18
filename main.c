/// @file main.c
#include <stdio.h>
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include <string.h>

#include "SDL/SDL_image.h"
#include "SDL/SDL_keysym.h"
#include "SDL/SDL_keyboard.h"
#include "SDL/SDL_events.h"
#include "SDL/SDL_rotozoom.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "chipmunk/chipmunk_private.h"
//#include "ChipmunkDemo.h"


#define max(a,b) (a>=b?a:b)
#define min(a,b) (a<=b?a:b)
#define abs(a) (a<0?-a:a)


#define PLAYER_VELOCITY 50.0

#define PLAYER_GROUND_ACCEL_TIME 0.1
#define PLAYER_GROUND_ACCEL (PLAYER_VELOCITY/PLAYER_GROUND_ACCEL_TIME)

#define PLAYER_AIR_ACCEL_TIME 0.25
#define PLAYER_AIR_ACCEL (PLAYER_VELOCITY/PLAYER_AIR_ACCEL_TIME)

#define JUMP_HEIGHT 100.0
#define JUMP_BOOST_HEIGHT 55.0
#define FALL_VELOCITY 900.0
#define GRAVITY -20.0

cpBody *playerBody = NULL;
cpBody *playerBody2 = NULL;
cpShape *playerShape = NULL;
cpShape *playerShape2 = NULL;
cpSpace *_space = NULL;



cpFloat remainingBoost = 0;
cpBool grounded = cpFalse;
int doubleJumped = 0;
cpBool lastJumpState = cpFalse;
cpVect moveDir;

int isMultiplayer = 0;

cpBool grounded2 = cpFalse;
int doubleJumped2 = 0;
cpBool lastJumpState2 = cpFalse;
cpVect moveDir2;
int pvelx = 0;
int pvely = 0;



enum CollisionTypes {
	COLLISION_TYPE_ONE_WAY = 1,
};

typedef struct OneWayPlatform {
	cpVect n; // direction objects may pass through
} OneWayPlatform;

typedef struct PlayerData {
	int id;
} PlayerData;

static OneWayPlatform platformInstance;
static PlayerData player1Data;
static PlayerData player2Data;

static cpBool
PreSolve(cpArbiter *arb, cpSpace *space, void *ignore)
{
  //printf("stage1\n");
	CP_ARBITER_GET_SHAPES(arb, a, b);
  //printf("stage2\n");
	OneWayPlatform *platform = (OneWayPlatform *)cpShapeGetUserData(a);
	PlayerData *cplayer = (PlayerData *)cpShapeGetUserData(b);
  //printf("stage3\n");
  if(platform==NULL)return cpTrue;
	if(cpvdot(cpArbiterGetNormal(arb), platform->n) < 0 && false){
    //printf("stage3a\n");
		printf("ignoring 3\n");
		return cpArbiterIgnore(arb);
	}
  //printf("stage4\n");

  if(cplayer->id==1 && (playerBody->v.y<0.0 || moveDir.y>0))
	{
		printf("ignoring 1\n");
		return cpArbiterIgnore(arb);
	}

  if(cplayer->id==2 && (playerBody2->v.y<0.0|| moveDir2.y>0))
	{
		printf("ignoring 2\n");
		return cpArbiterIgnore(arb);
	}

	return cpTrue;
}

/*
#define GRABBABLE_MASK_BIT (1<<31)
cpShapeFilter GRAB_FILTER = {CP_NO_GROUP, GRABBABLE_MASK_BIT, GRABBABLE_MASK_BIT};
cpShapeFilter NOT_GRABBABLE_FILTER = {CP_NO_GROUP, ~GRABBABLE_MASK_BIT, ~GRABBABLE_MASK_BIT};
*/
//#include "SDL2/SDL_image.h"
/**
* Structure Sprite qui define tout type d'objets à afficher sur l'ecran (entités, boutons, background, etc)
*/
typedef struct _s
{

  SDL_Surface* image; /**< image du sprite par defaut */
  SDL_Surface* hoverImage; /**< image du bouton lors du mouvement de la souris par dessus */
  SDL_Surface* clickImage; /**< image du bouton lors de l'appui de la souris */
  SDL_Rect pos; /**< la position (sur ecran si isStatic==1, dans le monde du jeu si isStatic==0) */

	SDL_Surface* anims[30]; /**< tableau des images de l'animation du sprite */
	int animated; /**< 1 si l'objet est animé, sinon 0 */
	int nanims; /**< nombre des images de l'animation */

  cpBody *body; /**< l'objet de collision relié au sprite */


  int clickEvent; /**< l'identifient lors du clic */
  int type; /**< le type (1=normal, 2=bouton) */
  int visible; /**< 1 si visible, sinon 0; Si c'est 0, le sprite sera pas affiché */
  int order; /**< l'ordre d'affichage du sprite */
  int menu; /**< l'identifiant du sous menu associé au sprite */
  int updateEvent; /**< l'identifiant pour faire des operations sur le sprite chaque iteration du jeu */
  int ignoreMouse; /**< si 1, les interactions avec la souris seront ignorés */
	int isStatic; /**< si 1, l'objet ne sera pas affecté par le zoom et le mouvement du joueur */
	int owner; /**< le proprietaire lors du jeu d'enigme contre l'IA2 */
	int used; /**< 1 si utilisé (ne sera plus possible d'appuyer dessus), sinon 0 */
} Sprite;

/**
* Structure Camera qui sert à mettre les images sur l'ecran relativement à la position de la camera
*/
typedef struct _camera
{
  int x;
  int y;

} Camera;


Sprite* Sprites[100];
Sprite* EnigmeButtons[100];
int EnigmeGrid[9] = {0,0,0,0,0,0,0,0,0};
Sprite* volSprite;
Sprite* playerSprite;
Sprite* playerSprite2;
Sprite* enemySprite;
Sprite* minimapicon;
Camera menuCamera;
Camera gameCamera;
int cEnigmePlayer = 0;
int nSprites = 0;
int gameRunning = 1;
int cMouseOver = -1;
int mouseDown = 0;
int processedClick = 0;
int cMenu = 0;
int isFullscreen = 0;
SDL_Surface *screen;
SDL_Surface *fscreen;
SDL_Surface *sscreen;
SDL_Surface *sscreen2;
SDL_Surface *tscreen;
int ticks = 0;

int lives = 3;
int lives2 = 3;

int cScene = 1;

void SelectPlayerGroundNormal(cpBody *body, cpArbiter *arb, cpVect *groundNormal){
	cpVect n = cpvneg(cpArbiterGetNormal(arb));

	if(n.y < groundNormal->y){
		(*groundNormal) = n;
	}
}

void SelectPlayerGroundNormal2(cpBody *body, cpArbiter *arb, cpVect *groundNormal){
	cpVect n = cpvneg(cpArbiterGetNormal(arb));

	if(n.y < groundNormal->y){
		(*groundNormal) = n;
	}
}


/**
 * Fonction executée chaque iteration pour verifier si le joueur est sur la plateforme pour assumer s'il peut faire saut ou non
 */
void playerUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	int jumpState = (moveDir.y < 0.0f);

	// Grab the grounding normal from last frame
	cpVect groundNormal = cpvzero;
	cpBodyEachArbiter(playerBody, (cpBodyArbiterIteratorFunc)SelectPlayerGroundNormal, &groundNormal);

	grounded = (groundNormal.y < 0.0);
  //printf("ground normal y = %.6f\n", groundNormal.y);
	//if(groundNormal.y > 0.0f) remainingBoost = 0.0f;

	//Do a normal-ish update
	cpBool boost = (jumpState && remainingBoost > 0.0f);
	cpVect g = (gravity);
	cpBodyUpdateVelocity(body, g, damping, dt);

	// Target horizontal speed for air/ground control
	cpFloat target_vx = PLAYER_VELOCITY*moveDir.x;
	if(lives<=0)target_vx = 0;

	// Update the surface velocity and friction
	// Note that the "feet" move in the opposite direction of the player.
	cpVect surface_v = cpv(-target_vx, 0.0);
	playerShape->surfaceV = surface_v;
	playerShape->u = (grounded ? (-PLAYER_GROUND_ACCEL/GRAVITY) : 0.0);

	// Apply air control if not grounded
	if(!grounded){
		// Smoothly accelerate the velocity
		playerBody->v.x = cpflerpconst(playerBody->v.x, target_vx, PLAYER_AIR_ACCEL*dt);
	}

	body->v.y = cpfclamp(body->v.y, -FALL_VELOCITY, INFINITY);
}


void playerUpdateVelocity2(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	int jumpState2 = (moveDir2.y < 0.0f);

	// Grab the grounding normal from last frame
	cpVect groundNormal = cpvzero;
	cpBodyEachArbiter(playerBody2, (cpBodyArbiterIteratorFunc)SelectPlayerGroundNormal2, &groundNormal);

	grounded2 = (groundNormal.y < 0.0);
  //printf("ground normal y = %.6f\n", groundNormal.y);
	//if(groundNormal.y > 0.0f) remainingBoost = 0.0f;

	//Do a normal-ish update
	cpBool boost = (jumpState2 && remainingBoost > 0.0f);
	cpVect g = (gravity);
	cpBodyUpdateVelocity(body, g, damping, dt);

	// Target horizontal speed for air/ground control
	cpFloat target_vx = PLAYER_VELOCITY*moveDir2.x;
	if(lives2<=0)target_vx = 0;

	// Update the surface velocity and friction
	// Note that the "feet" move in the opposite direction of the player.
	cpVect surface_v = cpv(-target_vx, 0.0);
	playerShape2->surfaceV = surface_v;
	playerShape2->u = (grounded2 ? (-PLAYER_GROUND_ACCEL/GRAVITY) : 0.0);

	// Apply air control if not grounded
	if(!grounded2){
		// Smoothly accelerate the velocity
		playerBody2->v.x = cpflerpconst(playerBody2->v.x, target_vx, PLAYER_AIR_ACCEL*dt);
	}

	body->v.y = cpfclamp(body->v.y, -FALL_VELOCITY, INFINITY);
}




/**
 * Fonction qui sert à initializer l'espace virtuel de la librairie de collision
 */
void initSpace()
{
	cpSpace *space = cpSpaceNew();
	space->iterations = 10;
	space->gravity = cpv(0, -GRAVITY);
	cpSpaceSetIterations(space, 100);
//	space->sleepTimeThreshold = 1000;

	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;

/*
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,0), cpv(800,0), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	//cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,0), cpv(0,480), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	//cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(800,480), cpv(0,480), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	//cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(800,480), cpv(800,0), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	//cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
*/





	// Set up the player
	body = cpSpaceAddBody(space, cpBodyNew(1.0f, INFINITY));
	body->p = cpv(200, 50);
	body->velocity_func = playerUpdateVelocity;
	playerBody = body;

	shape = cpSpaceAddShape(space, cpBoxShapeNew2(body, cpBBNew(0.0, 60.0, 50.0, 0.0), 10.0));
	//	shape = cpSpaceAddShape(space, cpSegmentShapeNew(playerBody, cpvzero, cpv(0, radius), radius));
	shape->e = 0.0f; shape->u = 0.0f;
	shape->type = 1;
	playerShape = shape;
	player1Data.id = 1;
	cpShapeSetUserData(playerShape, &player1Data);


	body = cpSpaceAddBody(space, cpBodyNew(1.0f, INFINITY));
	body->p = cpv(600, 50);
	body->velocity_func = playerUpdateVelocity2;
	playerBody2 = body;

	shape = cpSpaceAddShape(space, cpBoxShapeNew2(body, cpBBNew(0.0, 60.0, 50.0, 0.0), 10.0));
//	shape = cpSpaceAddShape(space, cpSegmentShapeNew(playerBody, cpvzero, cpv(0, radius), radius));
	shape->e = 0.0f; shape->u = 0.0f;
	shape->type = 1;
	playerShape2 = shape;
	player2Data.id = 2;
	cpShapeSetUserData(playerShape2, &player2Data);



	// Add some boxes to jump on
	/*for(int i=0; i<6; i++){
		for(int j=0; j<3; j++){
			body = cpSpaceAddBody(space, cpBodyNew(4.0f, INFINITY));
			body->p = cpv(100 + j*60, -200 + i*60);

			shape = cpSpaceAddShape(space, cpBoxShapeNew(body, 50, 50, 0.0));
			shape->e = 0.0f; shape->u = 0.7f;
		}
	}*/

/*
  cpSpaceSetCollisionSlop(space, 1);
  cpSpaceSetCollisionBias(space, 0.0005);
  cpSpaceSetCollisionPersistence(space, 10);
*/

  cpCollisionHandler *handler = cpSpaceAddWildcardHandler(space, COLLISION_TYPE_ONE_WAY);
  handler->preSolveFunc = PreSolve;
  platformInstance.n = cpv(0, -1); // let objects pass upwards

	_space = space;
}


/**
 * fonction qui sert à créer un nouveau sprite et l'ajouter au tableau des sprites pour l'afficher sur l'ecran
 * @param name le nom de l'image du sprite
 * @param type le type du sprite (1=normal, 2=bouton)
 */
Sprite* MakeSprite(char name[], int type)
{
  char fname[40] = "";
  strcpy(fname, name);
  strcat(fname, ".png");
  //printf("name = %s\n", fname);
  Sprite* sprite = malloc(sizeof(Sprite));
  sprite->image = IMG_Load(fname);
  sprite->visible = 1;
  sprite->ignoreMouse = 0;
  sprite->type=type;
  sprite->clickEvent = -1;
	sprite->isStatic = 0;
	sprite->owner = 0;
	sprite->used = 0;
  if(type==1)
  {
    char hname[30] = "";
    strcat(hname, name);
    strcat(hname, " hover.png");
    sprite->hoverImage = IMG_Load(hname);
    char cname[30] = "";
    strcat(cname, name);
    strcat(cname, " click.png");
    sprite->clickImage = IMG_Load(cname);

    if(sprite->hoverImage==NULL)
    {
      printf("Unable to load hover bitmap: %s\n", SDL_GetError());
    }

    if(sprite->clickImage==NULL)
    {
      printf("Unable to load click bitmap: %s\n", SDL_GetError());
    }

  }
  //image = SDL_LoadBMP("images/background main.bmp");
  if(sprite->image==NULL)
  {
    printf("Unable to load bitmap: %s\n", SDL_GetError());
  }
  printf("error : %s\n", SDL_GetError());
  Sprites[nSprites++] = sprite;
  return sprite;
}

/**
 * fonction qui verifie si la position de la souris est sur un bouton
 */
void checkMouse()
{
  cMouseOver = -1;
  int mx,my;
  SDL_GetMouseState(&mx, &my);
  for(int o=nSprites-1; o>=0; o--)
  {
    if( (Sprites[o]->menu != cMenu && Sprites[o]->menu!=-1) || Sprites[o]->visible == 0 || Sprites[o]->ignoreMouse == 1) continue;
    if(Sprites[o]->pos.x<=mx && Sprites[o]->pos.y<=my && Sprites[o]->pos.x+Sprites[o]->pos.w>=mx && Sprites[o]->pos.y+Sprites[o]->pos.h>=my)
    {
      cMouseOver = o;
      break;
    }
  }
}

/**
 * fonction qui sert à créer un nouveau bouton et l'ajouter au tableau des sprites pour l'afficher sur l'ecran
 * @param name le nom de l'image du bouton
 * @param x position sur l'axe horizontal du bouton
 * @param y position sur l'axe vertical du bouton
 * @param m l'identifiant du sous menu associé au bouton
 */
Sprite* addButton(char name[], int x, int y, int m)
{
  Sprite* btn = MakeSprite(name, 1);
  btn->pos.x = x;
  btn->pos.y = y;
  btn->pos.w = btn->image->w;
  btn->pos.h = btn->image->h;
  btn->menu = m;

  return btn;
}

/**
 * fonction qui sert à créer un nouveau sprite et l'ajouter au tableau des sprites pour l'afficher sur l'ecran
 * @param name le nom de l'image du sprite
 * @param x position sur l'axe horizontal du sprite
 * @param y position sur l'axe vertical du sprite
 * @param m l'identifiant du sous menu associé au sprite
 */
Sprite* addSprite(char name[], int x, int y, int m)
{
  Sprite* btn = MakeSprite(name, 0);
  btn->pos.x = x;
  btn->pos.y = y;
  btn->menu = m;
  btn->body = NULL;

  return btn;
}

/**
 * ajoute un objet immobile à l'espace virtual de collision (principalement pour la collision avec la plateforme)
 * @param space reference de l'espace virtuel de la librairie de collision
 * @param px position sur l'axe horizontal de l'objet
 * @param py position sur l'axe vertical de l'objet
 * @param sx largeur de l'objet
 * @param sy hauteur de l'objet
 */
void addBox(cpSpace *space, int px, int py, int sx, int sy)
{
	px-=10;
  cpBody *staticBody = cpSpaceGetStaticBody(space);//cpSpaceAddBody(space, cpBodyNewStatic());
  //staticBody->p = cpv(px+sx/2, py+sy/2);

  cpShape *shape;


	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(px,py+sy/2), cpv(px+sx,py+sy/2), 0.0f));
	//shape = cpSpaceAddShape(space, cpBoxShapeNew(staticBody, sx/2, sy/2, 0));
	shape->e = 1.0f; shape->u = 1.0f;

	cpShapeSetCollisionType(shape, COLLISION_TYPE_ONE_WAY);

	cpShapeSetUserData(shape, &platformInstance);

/*
  shape = cpSpaceAddShape(space, cpBoxShapeNew(staticBody, sx/2, sy/2, 0));
  shape->e = 1.0f; shape->u = 1.0f;
*/
  //cpSpaceReindexStatic(space);
  //cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
  //addSprite("images/vol", 0, 0, 0)->body = staticBody;
}

/**
 * joue un morceau de musique
 * @param name le nom du ficher son
 * @param rep le nombre de repetitions du son
 */
void PlaySound(char name[], int rep)
{
  Mix_Music *music;
  music = Mix_LoadMUS(name);
  Mix_PlayMusic(music, rep);
}


/**
 * joue un son bref
 * @param name le nom du ficher son
 * @param rep le nombre de repetitions du son
 */
void PlaySoundSingle(char name[], int rep)
{
  Mix_Chunk *music;
  music = Mix_LoadWAV(name);

  Mix_PlayChannel(-1, music, rep);
}

/**
 * Verifie s'il y a un gagnant dans l'enigme TicTacToe
 * @param nempty le nombre des cases vides restantes dans l'enigme TicTacToe
 */
void checkEnigmeWin(int nempty)
{
	int winner = 0;
	int combos[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
	for(int i=0; i<8; i++)
	{
		if(EnigmeGrid[combos[i][0]] != 0 && EnigmeGrid[combos[i][0]] == EnigmeGrid[combos[i][1]] && EnigmeGrid[combos[i][0]] == EnigmeGrid[combos[i][2]])
		{
			winner = EnigmeGrid[combos[i][0]];
		}
	}
	if(winner!=0)
	{
		printf("winner = %d\n", winner);
		if(winner==1)
		{
			if(cEnigmePlayer==1)
			{
				playerBody->p.x = 200;
	      playerBody->p.y = 0;
				playerSprite->visible = 1;
				lives++;
			}
			else if(cEnigmePlayer==2 || nempty == 0)
			{
				playerBody2->p.x = 600;
	      playerBody2->p.y = 50;
	      playerBody2->v.y = 0;
				playerSprite2->visible = 1;
				lives2++;
			}
			cMenu = 1;
		}
		else
		{
			setScene(0);
			cMenu = 0;
		}
		for(int i=0; i<9; i++)
		{
			EnigmeButtons[i]->used = 0;
			EnigmeButtons[i]->image = IMG_Load("images/empty.png");
			EnigmeButtons[i]->hoverImage = IMG_Load("images/empty hover.png");
			EnigmeButtons[i]->clickImage = IMG_Load("images/empty click.png");
			EnigmeGrid[i] = 0;
		}
	}
}



/**
 * Executée lors d'un clic sur un bouton dons l'identifiant de clic est non nul
 * @param sprite la reference du bouton appuyé dessus
 * @param e l'identifiant du clic déclaré dans la sprite
 */
void processClickEvent(Sprite* sprite, int e)
{
  printf("clicked : %d \n", e);
  switch(e)
  {
    case 0:
      {
        //SDL_FreeSurface(screen);
        cMenu = 1;
        break;

      }
			case 1:
	      {
					isMultiplayer = 0;
					setScene(1);
	        //Sprites[0]->image = SDL_LoadBMP("images/background main 2.bmp");
	        break;
	      }
			case 111:
	      {
					isMultiplayer = 1;
					setScene(1);
	        //Sprites[0]->image = SDL_LoadBMP("images/background main 2.bmp");
	        break;
	      }
    case 2:
      {
        cMenu = 2;
        break;
      }
    case 3:
    {
      if(isFullscreen==1)
      {

        isFullscreen = 0;
        screen = SDL_SetVideoMode(800, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

      }
      else
      {

        isFullscreen = 1;
        screen = SDL_SetVideoMode(800, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);

      }

      //cMenu = 1;
      break;
    }
    case 5:
      cMenu = 0;
      break;
    case 6:
    {
      int mx,my;
      SDL_GetMouseState(&mx, &my);
      mx-=25;
      if(mx<330)mx=330;
      if(mx>500)mx=500;

      volSprite->pos.x = mx;
      Mix_Volume(-1,(int)(MIX_MAX_VOLUME*(mx-330.0)/170.0));
      Mix_VolumeMusic((int)(MIX_MAX_VOLUME*(mx-330.0)/170.0));
      break;
    }
    case 7:
    {
      cMenu = 4;
      break;
    }
    case 8:
      cMenu = 3;
      break;
    case 9:
      {
        gameRunning = 0;
        break;
      }
		default:
		{
			if(e>=60)
			{
				if(sprite->used == 0)
				{
					sprite->used = 1;
					sprite->image = IMG_Load("images/cross.png");
					sprite->hoverImage = IMG_Load("images/cross.png");
					sprite->clickImage = IMG_Load("images/cross.png");

					EnigmeGrid[e-61] = 1;

					srand(time(0));
					int rands[9];
					int nrands = 0;
					for(int i=0; i<9; i++)
					{
						if(EnigmeGrid[i]==0)
						{
							rands[nrands++] = i;
						}
					}
					printf("nrands : %d\n", nrands);
					checkEnigmeWin(nrands);
					if(nrands>0)
					{
						int crand = rands[rand()%nrands];
						EnigmeButtons[crand]->used = 1;
						EnigmeButtons[crand]->image = IMG_Load("images/circle.png");
						EnigmeButtons[crand]->hoverImage = IMG_Load("images/circle.png");
						EnigmeButtons[crand]->clickImage = IMG_Load("images/circle.png");
						EnigmeGrid[crand] = 2;
						checkEnigmeWin(nrands-1);
					}

				}
			}

		}
  }
}


/**
 * Executée chaque iteration du jeu sur les sprites dont l'identifiant updateEvent est non nul
 * @param sprite la reference de la sprite
 * @param e l'identifiant du sprite
 */
void processUpdate(Sprite* sprite, int event)
{
  switch(event)
  {
    case 1:
    {
      sprite->pos.x = (400+ticks*10)%800;
      sprite->pos.y = (240+ticks*10)%480;
      break;
    }
    case 2:
    {
      if(sprite->pos.y>-500)sprite->pos.y-=5;
      break;
    }
  }
}

/**
 * Change la scene (0 = menu principal, 1 = jeu)
 * @param id l'identifiant de la scene
 */
void setScene(int id)
{
  nSprites = 0;
	cScene = id;
  if(id==0)
  {
    Sprite* bg = addSprite("images/background main", 230, 100, -1);
    bg->pos.x = 0;
    bg->pos.y = 0;
    bg->pos.w = 800;
    bg->pos.h = 480;
    //bg->visible = 0;

    addSprite("images/Bomb6", 230, 100, -1)->updateEvent=1;


    int btnx = 300;
    int btny = 100;
    int btnh = 70;

    addButton("images/new game", btnx, btny, 0)->clickEvent = 0;
    Sprite* contBtn = addButton("images/continue", btnx, btny+btnh, 0);
    contBtn->clickEvent = 1;
    contBtn->visible = 1;

    addButton("images/vol", 800-60, 10, -1)->clickEvent = 3;



    addButton("images/settings", btnx, btny+btnh*2, 0)->clickEvent = 2;
    addButton("images/CRED", btnx, btny+btnh*3, 0)->clickEvent = 7;
    addButton("images/quit", btnx, btny+btnh*4, 0)->clickEvent = 8;



    addButton("images/solo", btnx, btny+btnh, 1)->clickEvent = 1;
    addButton("images/multiplayer", btnx, btny+btnh*2, 1)->clickEvent = 111;
    addButton("images/quit", btnx, btny+btnh*3, 1)->clickEvent = 5;

    addSprite("images/pop up menu", 230, 100, 2);
    addButton("images/volume", btnx, btny+btnh, 2)->clickEvent = 6;
    volSprite = addSprite("images/vol", 330, btny+btnh+10, 2);
    volSprite->ignoreMouse = 1;
    addButton("images/quit", btnx, btny+btnh*3, 2)->clickEvent = 5;

    addSprite("images/quit popup", 230, 100, 3);
    addButton("images/yes", 150, 350, 3)->clickEvent = 9;
    addButton("images/No", 450, 350, 3)->clickEvent = 5;
    //addButton("images/quit", btnx, btny+btnh*3, 2)->clickEvent = 5;

    Sprite* creds = addSprite("images/credits", 230, 100, 4);
    creds->updateEvent = 2;
    addButton("images/quit", 800-200, 480-100, 4)->clickEvent = 5;



    //SDL_Delay(1000);
    PlaySound("audio/music.mp3", -1);
  }
  else if(id==1)
  {
		lives = 3;
		lives2 = 3;
    initSpace();
		addSprite("images/gamebg1", 0, 0, -1);
		Sprite* minimap = addSprite("images/minimap", 0, 0, -1);
		minimap->isStatic = 1;
		minimap->pos.y=480-106;

		minimapicon = addSprite("images/minimapicon", 0, 0, -1);
		minimapicon->isStatic = 1;

    addBox(_space, 140, 170, 200, 10);
    addBox(_space, 535, 170, 200, 10);
    addBox(_space, 325, 260, 250, 10);
    addBox(_space, 140, 370, 200, 10);
    addBox(_space, 535, 370, 200, 10);

		playerSprite = addSprite("images/male1", 0, 0, -1);
		playerSprite->anims[0] = IMG_Load("images/male1.png");
		playerSprite->anims[1] = IMG_Load("images/male2.png");
		playerSprite->anims[2] = IMG_Load("images/male3.png");
		playerSprite->anims[3] = IMG_Load("images/male4.png");


		playerSprite2 = addSprite("images/female1", 0, 0, -1);
		playerSprite2->anims[0] = IMG_Load("images/female1.png");
		playerSprite2->anims[1] = IMG_Load("images/female2.png");
		playerSprite2->anims[2] = IMG_Load("images/female3.png");
		playerSprite2->anims[3] = IMG_Load("images/female4.png");

		if(isMultiplayer==0)
		{
			playerSprite2->visible = 0;
			lives2 = 0;
		}
		enemySprite = addSprite("images/potato1", 0, 0, -1);

		enemySprite->anims[0] = IMG_Load("images/potato1.png");
		enemySprite->anims[1] = IMG_Load("images/potato2.png");
		enemySprite->anims[2] = IMG_Load("images/potato3.png");

		showEnigme();

  }
}


/**
 * met l'image sur l'ecran soit relativement à l'ecran (isSTatic = 1), soit relativement au monde du jeu (isStatic = 0)
 */
void blitSurface(SDL_Surface *img, SDL_Rect *pos1, SDL_Surface *target, SDL_Rect *pos2, int isStatic)
{

  SDL_Rect fpos = (*pos2);
	if(isStatic==0)
	{
		fpos.x = fpos.x-gameCamera.x+400;
		fpos.y = fpos.y-gameCamera.y+240;
	}

	SDL_BlitSurface(img, pos1, target, &fpos);

}


/**
 * Fonction qui initialize l'enigme TicTacToe contre l'IA2
 */
void showEnigme()
{
	Sprite* bgSprite = addSprite("images/tictactoebg", 400-206, 240-206, 3);
	bgSprite->isStatic = 1;
	for(int y=0; y<3; y++)
	for(int x=0; x<3; x++)
	{
		Sprite* button1x1 = addButton("images/empty", 400-206+101+80*x, 240-206+101+80*y, 3);
		button1x1->isStatic = 1;
		button1x1->clickEvent = 61+x+y*3;
		EnigmeButtons[x+y*3] = button1x1;
	}
}



/**
 * Fonction principale du programme du jeu qui contient le boucle du jeu
 */
int main()
{


  SDL_Delay(1000);
  SDL_Surface *image;
  SDL_Rect positionecran;
  char pause;
  int flags=IMG_INIT_JPG|IMG_INIT_PNG;
  int initted=IMG_Init(flags);
  if(SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  screen = SDL_SetVideoMode(800, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );

		int rmask,gmask,bmask,amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

sscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 800, 480, 32, rmask, gmask, bmask, amask);
sscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, 800, 480, 32, rmask, gmask, bmask, amask);
		fscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 800, 480, 32, rmask, gmask, bmask, amask);
		if(fscreen == NULL) {
        printf("CreateRGBSurface failed: %s\n", SDL_GetError());
				//exit(1);
    }
		if(sscreen == NULL) {
        printf("CreateRGBSurface 2 failed: %s\n", SDL_GetError());
				//exit(1);
    }


  if(screen==NULL)
  {
    printf("Unable to set video mode : %s\n", SDL_GetError());
    return 1;
  }

  Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);


  setScene(0);

  printf("finished loading images.\n");
  while(gameRunning==1)
  {

    //continueBtn.pos.x++;
    double dt = 0.2;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        gameRunning = 0;
      }
      else if(event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONDOWN)
      {
        mouseDown = 1;
      }
      else if(event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONUP)
      {
        mouseDown = 0;
        processedClick = 0;
      }
      else if(cScene==1 && (event.type==SDL_KEYDOWN || event.type==SDL_KEYUP))
      {
        //printf("yes pressed : %d\n", event.key.keysym.scancode);
        switch(event.key.keysym.sym){
					case SDLK_z    : moveDir.y += (event.type == SDL_KEYDOWN ?  -1.0 : 1.0); break;
    			case SDLK_s  : moveDir.y += (event.type == SDL_KEYDOWN ? 1.0 :  -1.0); break;
    			case SDLK_q  : moveDir.x += (event.type == SDL_KEYDOWN ? -1.0 :  1.0); break;
    			case SDLK_d : moveDir.x += (event.type == SDL_KEYDOWN ?  1.0 : -1.0); break;
					case SDLK_UP    : moveDir2.y += (event.type == SDL_KEYDOWN ?  -1.0 : 1.0); break;
    			case SDLK_DOWN  : moveDir2.y += (event.type == SDL_KEYDOWN ? 1.0 :  -1.0); break;
    			case SDLK_LEFT  : moveDir2.x += (event.type == SDL_KEYDOWN ? -1.0 :  1.0); break;
    			case SDLK_RIGHT : moveDir2.x += (event.type == SDL_KEYDOWN ?  1.0 : -1.0); break;
    			default: break;
    		}
      }
      else
      {
        //printf("another event.\n");
      }

    }


		if(lives<=0)
		{
			moveDir.x = 0;
			moveDir.y = 0;
		}

		if(lives2<=0)
		{
			moveDir2.x = 0;
			moveDir2.y = 0;
		}


		float cZoom = 1;
		//printf("milestone 1");
		if(cScene==1)
		{
			int potatoNum = (ticks/10)%3;

			int playerNum = (ticks/2)%4;



			playerSprite->image = playerSprite->anims[moveDir.x==0?0:playerNum];

			playerSprite2->image = playerSprite2->anims[moveDir2.x==0?0:playerNum];

			enemySprite->image = enemySprite->anims[potatoNum];

			pvelx=min(5, max(-5, pvelx+((playerSprite->pos.x-enemySprite->pos.x>0)?1:-1)));
			pvely=min(5, max(-5, pvely+((playerSprite->pos.y-enemySprite->pos.y>0)?1:-1)));
			if(ticks%2)
			{
				enemySprite->pos.x+=pvelx;
				enemySprite->pos.y+=pvely;
			}

			int jumpState = (moveDir.y < 0.0f);
	    //printf("jump state = %d\n", jumpState);
	    //printf("grounded = %d\n", grounded);

	  	// If the jump key was just pressed this frame, jump!
	  	if(jumpState && !lastJumpState && (grounded || !doubleJumped)) {
	  		cpFloat jump_v = -cpfsqrt(2.0*JUMP_HEIGHT*GRAVITY*-1);
	      //printf("f = %.5f\n", jump_v);
	      playerShape->u = (0.0);
	  		playerBody->v = cpvadd(playerBody->v, cpv(0.0, jump_v-playerBody->v.y));
	      if(!grounded)doubleJumped = 1;
	      else doubleJumped = 0;
	  		//remainingBoost = JUMP_BOOST_HEIGHT/jump_v;
	  	}

	  	//remainingBoost -= dt;
	  	lastJumpState = jumpState;

			if(playerBody->p.y>600)
	    {
	      playerBody->p.x = 200;
	      playerBody->p.y = 0;
	      lives--;
				if(lives==0)
				{
					cMenu = 3;
					cEnigmePlayer = 1;
				}
	    }


			int jumpState2 = (moveDir2.y < 0.0f);
	    //printf("jump state = %d\n", jumpState);
	    //printf("grounded = %d\n", grounded);

	  	// If the jump key was just pressed this frame, jump!
			if(jumpState2 && !lastJumpState2 && (grounded2 || !doubleJumped2)) {
	  		cpFloat jump_v = -cpfsqrt(2.0*JUMP_HEIGHT*GRAVITY*-1);
	      //printf("f = %.5f\n", jump_v);
	      playerShape2->u = (0.0);
	  		playerBody2->v = cpvadd(playerBody2->v, cpv(0.0, jump_v-playerBody2->v.y));
	      if(!grounded2)doubleJumped2 = 1;
	      else doubleJumped2 = 0;
	  		//remainingBoost = JUMP_BOOST_HEIGHT/jump_v;
	  	}

	  	// Step the space
			for(int s=0; s<1; s++)
	  	cpSpaceStep(_space, dt);

	  	//remainingBoost -= dt;
	  	lastJumpState2 = jumpState2;

			if(playerBody2->p.y>600 && lives2>0)
	    {
	      playerBody2->p.x = 600;
	      playerBody2->p.y = 50;
	      playerBody2->v.y = 0;
	      lives2--;
				if(lives2<=0)
				{
					cMenu = 3;
					printf("lives2 = %d\n", lives2);
					cEnigmePlayer = 2;
				}
			}

			if(lives>0)
			{
				playerSprite->pos.x = floor(playerBody->p.x);
	  		playerSprite->pos.y = floor(playerBody->p.y);
			}
			else
			{
				playerSprite->visible = 0;

				playerSprite->pos.x = 400;
	  		playerSprite->pos.y = 240;
			}
			if(lives2>0)
			{
				playerSprite2->pos.x = floor(playerBody2->p.x);
	  		playerSprite2->pos.y = floor(playerBody2->p.y);
			}
			else
			{
				playerSprite2->visible = 0;

				playerSprite2->pos.x = 400;
	  		playerSprite2->pos.y = 240;
			}
			minimapicon->pos.y = 480-100+playerSprite->pos.y/7;
			minimapicon->pos.x = playerSprite->pos.x/8;
	  	//printf("pos = %.2f , %.2f\n", playerBody->p.x, playerBody->p.y);


			float xdiff = playerSprite->pos.x+100;
			float ydiff = playerSprite->pos.y+100;
			if(xdiff<0)xdiff=-xdiff;
			if(ydiff<0)ydiff=-ydiff;
			//xdiff = xdiff*xdiff;
			//ydiff = ydiff*ydiff;

			gameCamera.x = (playerSprite->pos.x+playerSprite2->pos.x+80)/2;
	    gameCamera.y = (playerSprite->pos.y+playerSprite2->pos.y+80)/2;

			float pDist = sqrt(xdiff+ydiff);
			float xzoom = 800.0/xdiff;
			float yzoom = 480.0/ydiff;
			cZoom = min(yzoom, xzoom);
			cZoom = min(cZoom, 1.5);
			cZoom = cZoom;
			//printf("czoom :\nx: %f\ny: %f\n", xzoom, yzoom);
			//cZoom = 1;

		}
		else if(cScene==0)
		{
			gameCamera.x = 400;
	    gameCamera.y = 240;
			cZoom = 1;
		}

    checkMouse();
    //printf("gonna print images.\n");

		SDL_FillRect(fscreen, NULL, SDL_MapRGB(fscreen->format, 0, 0, 0));




    //printf("nsprites = %d\n", nSprites);
    for(int o=0; o<nSprites; o++)
    {
			if(Sprites[o]->isStatic==1) continue;
      //printf("o = %d\n", o);
      if(Sprites[o]->updateEvent != -1)
        processUpdate(Sprites[o], Sprites[o]->updateEvent);
      if(Sprites[o]->visible != 1) continue;
      if(Sprites[o]->type==1)
      {
        //printf("o = %d\n", o);
        if(Sprites[o]->menu!=cMenu && Sprites[o]->menu!=-1)continue;
        if(cMouseOver==o)
        {
          //printf("o = %d\n", o);
          if(mouseDown==1)
          {

            blitSurface(Sprites[o]->clickImage, NULL, fscreen, &Sprites[o]->pos, Sprites[o]->isStatic);

            if(processedClick==0)
            {
              processedClick = 1;
              PlaySoundSingle("audio/click.ogg", 0);
              if(Sprites[o]->clickEvent != -1)
                processClickEvent(Sprites[o], Sprites[o]->clickEvent);
            }
          }
          else
            blitSurface(Sprites[o]->hoverImage, NULL, fscreen, &Sprites[o]->pos, Sprites[o]->isStatic);
        }
        else
        {
          //printf("printing : %d\n", o);
          blitSurface(Sprites[o]->image, NULL, fscreen, &Sprites[o]->pos, Sprites[o]->isStatic);
        }
      }
      else
      {
        //printf("printing bg");
        if(Sprites[o]->menu!=cMenu && Sprites[o]->menu!=-1 )
        {
          //printf("menu = %d\n", Sprites[o]->menu);
          continue;
        }
        if(Sprites[o]->body != NULL)
        {
          Sprites[o]->pos.x = Sprites[o]->body->p.x;
          Sprites[o]->pos.y = Sprites[o]->body->p.y;
        }
        blitSurface(Sprites[o]->image, NULL, fscreen, &Sprites[o]->pos, Sprites[o]->isStatic);

      }

      //printf("printed one\n");
    }
//SDL_BlitSurface(SDL_LoadBMP("images/background main.bmp"), NULL, screen, NULL);

    //SDL_BlitSurface(Sprites[o].hoverImage, &Sprites[o].pos, screen, &Sprites[o].pos);




		//printf("setting screen now\n");
		//SDL_FillRect(fscreen, NULL, SDL_MapRGB(fscreen->format, 0, 0, 0));
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

		if(cScene==0)
			tscreen = rotozoomSurface(fscreen, 0, 1, 0);
		else if(cScene==1)
		{

			tscreen = rotozoomSurface(fscreen, 0, cZoom, 0);
		}

		SDL_Rect zoomRect;
		zoomRect.x = (800*cZoom-800)/2;
		zoomRect.y = (480*cZoom-480)/2;
		zoomRect.w = 800;
		zoomRect.h = 480;
		SDL_BlitSurface(tscreen, &zoomRect, screen, NULL);

		for(int o=0; o<nSprites; o++)
    {
			if(Sprites[o]->isStatic==0) continue;
      //printf("o = %d\n", o);
      if(Sprites[o]->updateEvent != -1)
        processUpdate(Sprites[o], Sprites[o]->updateEvent);
      if(Sprites[o]->visible != 1) continue;
      if(Sprites[o]->type==1)
      {
        //printf("o = %d\n", o);
        if(Sprites[o]->menu!=cMenu && Sprites[o]->menu!=-1)continue;
        if(cMouseOver==o)
        {
          //printf("o = %d\n", o);
          if(mouseDown==1)
          {

            blitSurface(Sprites[o]->clickImage, NULL, screen, &Sprites[o]->pos, Sprites[o]->isStatic);

            if(processedClick==0)
            {
              processedClick = 1;
              PlaySoundSingle("audio/click.ogg", 0);
              if(Sprites[o]->clickEvent != -1)
                processClickEvent(Sprites[o], Sprites[o]->clickEvent);
            }
          }
          else
            blitSurface(Sprites[o]->hoverImage, NULL, screen, &Sprites[o]->pos, Sprites[o]->isStatic);
        }
        else
        {
          //printf("printing : %d\n", o);
          blitSurface(Sprites[o]->image, NULL, screen, &Sprites[o]->pos, Sprites[o]->isStatic);
        }
      }
      else
      {
        //printf("printing bg");
        if(Sprites[o]->menu!=cMenu && Sprites[o]->menu!=-1 )
        {
          //printf("menu = %d\n", Sprites[o]->menu);
          continue;
        }
        if(Sprites[o]->body != NULL)
        {
          Sprites[o]->pos.x = Sprites[o]->body->p.x;
          Sprites[o]->pos.y = Sprites[o]->body->p.y;
        }
        blitSurface(Sprites[o]->image, NULL, screen, &Sprites[o]->pos, Sprites[o]->isStatic);

      }

      //printf("printed one\n");
    }

    SDL_Flip(screen);

		SDL_Delay(16);
    ticks++;


		//SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

		//SDL_FreeSurface(sscreen);
		//SDL_FreeSurface(fscreen);


  }

  SDL_Quit();

  printf("done\n");

  //SDL_FreeSurface(image);
  //pause = getchar();

  printf("yeh worked xd : %s\n", SDL_GetError());

  return 0;
}
