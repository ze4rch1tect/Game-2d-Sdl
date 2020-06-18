prog:fonctions.o main.o
	gcc fonctions.o main.o -o prog -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx -lSDL_mixer -ldl -lX11 -lm -lpthread libchipmunk.a -lm
main.o:main.c
	gcc -c main.c -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx -lSDL_mixer -ldl -lX11 -lm -lpthread libchipmunk.a -lm
fonctions.o:fonctions.c
	gcc -c fonctions.c -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx -lSDL_mixer -ldl -lX11 -lm -lpthread libchipmunk.a -lm
