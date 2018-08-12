#include "SDL2/SDL.h"
#include "errno.h"
#include "stdbool.h"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// display
// original display = 64x32 mode, super chip8 = 128x64 mode
//
// -----------------
// sprites
// chip8 sprites may be up to 15 bytes
//
//


uint8_t ram[4096];

// Relevant to CPU
// Initialize

#define REGISTER (*(volatile uint8_t*)0x10000)
uint16_t pc;
uint16_t stack[16];
uint8_t sp;

uint16_t Vx[16];
uint16_t I;
uint16_t delay_timer;
uint16_t snd_timer;

//memory should be an 0xffff array? o-o

int initSDL();
void push(uint16_t instruction);
bool pop(uint16_t *val);

errno_t main() {
	//initSDL();
    pc = 0x200;
    push(0);
    push(1);
    push(2);

    uint16_t val;
    while ((val = pop(&val))) {
        return 0;
    }

	printf ("hello");
	return 0;
}	

bool pop(uint16_t *val) {
    return false;
}

void push(uint16_t instruction) { 
    stack[sp] = instruction;
    sp++;
}

errno_t execute () {
    return 0;
}

int test() {
    printf ("testing stack...");
    pc = 0x0;
    return 0;
}

int initSDL() {
	//The window we'll be rendering to
	SDL_Window* window = NULL;
	
	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//The image we will load and show on the screen
	SDL_Surface* gHelloWorld = NULL;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	}
	else
	{
		//Create window
		window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface( window );

			//Fill the surface white
			SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0x00, 0x00, 0x00 ) );
			
			// img pls
			gHelloWorld = SDL_LoadBMP( "hello_world.bmp" );
			if ( gHelloWorld == NULL ) {
				printf( "ree" );
			}

			SDL_BlitSurface( gHelloWorld, NULL, screenSurface, NULL );
			
			//Update the surface
			SDL_UpdateWindowSurface( window );

			//Wait two seconds
			SDL_Delay( 5000 );
		}
	}
	//Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
