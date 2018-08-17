#include "SDL2/SDL.h"
#include "stdlib.h"
#include "stdbool.h"

//Screen dimension constants
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

// display
// original display = 64x32 mode, super chip8 = 128x64 mode
// -----------------
// sprites
// chip8 sprites may be up to 15 bytes
//

static uint8_t ram[0x1000];

/* +---------------+= 0xFFF (4095) End of Chip-8 RAM
 * |               |
 * |               |
 * |               |
 * |               |
 * |               |
 * | 0x200 to 0xFFF|
 * |     Chip-8    |
 * | Program / Data|
 * |     Space     |
 * |               |
 * |               |
 * |               |
 * +- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
 * |               |
 * |               |
 * |               |
 * +---------------+= 0x200 (512) Start of most Chip-8 programs
 * | 0x000 to 0x1FF|
 * | Reserved for  |
 * |  interpreter  |
 * +---------------+= 0x000 (0) Start of Chip-8 RAM
 */


// Relevant to CPU
// Initialize

#define REGISTER (*(volatile uint8_t*)0x10000)
static uint16_t pc;
static uint16_t stack[16];
static int sp;

static uint16_t V[16];
static uint16_t I;
static uint16_t delay_timer;
static uint16_t snd_timer;

static int panel[64][32];

// Coordinate system:
// -------------------------
// | (0,0)        (63,0)   |
// |                       |
// | (0,31)       (63,31)  |
// -------------------------



//memory should be an 0xffff array? o-o

int initSDL();
void draw_SDL_panel();
void push(uint16_t instruction);
bool pop(uint16_t *instruction);
bool load_rom(const char* rom);

errno_t main() {
    
    errno_t ret = EXIT_SUCCESS;
    
    load_rom("wipeoff.rom");
    
    //const char* blah = "hello";
    //printf( "blah %s this is size of blah %d\n", blah, sizeof(*blah) );
    
	initSDL();
    pc = 0x200;
    sp = 0;


    uint16_t opcode;
    while ( true ) {
        bool should_continue = pop(&opcode);
        printf ("this is my opcode %d\n", opcode);

        if (!should_continue) {
            break;
        }
    }

	printf ("The program has finished.\n");
	return ret;
}	

bool load_rom(const char* rom) {
    char *buffer;
    long filelen;
    FILE* f_rom = fopen("/Users/Rich/8outof8_test/8outof8/8outof8/wipeoff.rom", "r+b");
    
    if (!f_rom) {
        return false; // opening file failed owo
    }
    
    fseek(f_rom, 0, SEEK_END);
    filelen = ftell(f_rom);
    rewind(f_rom);
    
    buffer = (char *) malloc( (filelen + 1) * sizeof(char) );
    fread(buffer, filelen, 1, f_rom);
    fclose(f_rom);
    
    for (int i = 0; i < filelen; i++) {
        uint16_t start = 0x200;
        ram[start+i] = *(buffer+i);
    }
    
    //printf("first slot: %x", ram[0x200]);
    //printf("file length: %d", (int) filelen);
    //printf("last slot: %x", ram[0x2cd]);
   
    return true;
}

bool pop(uint16_t *instruction) {
    //printf( "value from stack: %d", stack[sp-1] );
    *instruction = stack[sp - 1];
    //printf( "new instruction value: %d", *instruction );
    sp--; // sp should never be 0 when this is called.. 
    printf ("this is my sp: %d", sp); 
    if (sp <= 0) {
        return false;
    }
    return true;
}

void push(uint16_t instruction) { 
    //printf ("sp: %d\n", sp);
    stack[sp] = instruction;
    //printf ("instruction: %d\n", instruction);
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
    
    // Initialize Renderer
    SDL_Renderer* renderer = NULL;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	}
	else
	{
		//Create window
		//window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        //Create window w/ renderer
        
        SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0/*SDL_WINDOW_FULLSCREEN*/, &window, &renderer);

		if( window == NULL || renderer == NULL)
		{
			printf( "Window or renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get window surface
			//screenSurface = SDL_GetWindowSurface( window );

			//Fill the surface white
			//SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0x00, 0x00, 0x00 ) );
			
            if (SDL_SetRenderDrawColor(renderer, 0, 100, 100, SDL_ALPHA_OPAQUE)) {
                printf("Failed to set render draw color.");
                exit(1);
            }
            
            SDL_Point points[100];
            //SDL_Point test_point = { .x = 8, .y=8 };
            
            for (int i = 0; i < 14; i++) {
                points[i].x = i;
                points[i].y = i;
            }
            
            //const SDL_Point* point_array = { points };
            
            SDL_RenderDrawPoints( renderer, points, 14);
            SDL_RenderPresent(renderer);
			// img pls
			//gHelloWorld = SDL_LoadBMP( "hello_world.bmp" );
			//if ( gHelloWorld == NULL ) {
			//	printf( "ree" );
			//}

			//SDL_BlitSurface( gHelloWorld, NULL, screenSurface, NULL );
			
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

void draw_sdl_panel() {
    
    return;
}
