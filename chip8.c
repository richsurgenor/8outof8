#include "SDL2/SDL.h"
#include "stdlib.h"
#include "stdbool.h"

//Screen dimension constants
const int WINDOW_SCALE = 10;
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

//The window we'll be rendering to
static SDL_Window* window = NULL;

//The surface contained by the window
static SDL_Surface* screenSurface = NULL;

//The image we will load and show on the screen
//static SDL_Surface* gHelloWorld = NULL;

// Initialize Renderer
static SDL_Renderer* renderer = NULL;

// SDL Event Handler
static SDL_Event e;



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

static uint8_t V[16];
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
void push(uint16_t pc_in);
bool pop(uint16_t *pc_in);
bool load_rom(const char* rom);
errno_t execute_instruction(uint16_t instruction);

errno_t main() {
    
    errno_t ret = EXIT_SUCCESS;
    
    load_rom("wipeoff.rom");
    
    //const char* blah = "hello";
    //printf( "blah %s this is size of blah %d\n", blah, sizeof(*blah) );
    
	initSDL();
    
    pc = 0x200;
    sp = 0;

    for (int i = 0; i < 7; i++) {
        push(i);
    }

	//Wait two seconds
    uint16_t opcode;
    
    int moving_example = 0;
    bool quit = false;
    
    // game loop
    while ( !quit ) {
        
        // event loop
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
        }
        
        //bool should_continue = pop(&opcode);
        
        printf ("this is my opcode %d\n", opcode);
        
        if( window == NULL || renderer == NULL)
		{
			printf( "Window or renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
            exit(1);
		}
		
        bool draw = false;
        if (draw) {
			//Get window surface
			//screenSurface = SDL_GetWindowSurface( window );

			//Fill the surface white
			//SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0x00, 0x00, 0x00 ) );
			
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            
            SDL_RenderClear(renderer); // sets background color
            
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
            
            SDL_Point points[100];
            //SDL_Point test_point = { .x = 8, .y=8 };
            
            for (int i = 0; i < 14; i++) {
                points[i].x = i+moving_example;
                points[i].y = i+moving_example;
            }
            
            //const SDL_Point* point_array = { points };
            
            SDL_RenderDrawPoints( renderer, points, 14);
            //SDL_RenderDrawPoints( renderer, points, 14);
            
            //SDL_BlitSurface( gXOut, NULL, gScreenSurface, NULL );
            SDL_RenderPresent(renderer);
			// img pls
			//gHelloWorld = SDL_LoadBMP( "hello_world.bmp" );
			//if ( gHelloWorld == NULL ) {
			//	printf( "ree" );
			//}

			//SDL_BlitSurface( gHelloWorld, NULL, screenSurface, NULL );
			
			//Update the surface
			SDL_UpdateWindowSurface( window );
		}
        
        uint16_t instruction;
        
        // retrieve next instruction
        instruction = ram[pc+0];
        instruction = instruction << 8;
        instruction |= ram[pc+1];
        
        execute_instruction(instruction);
        
        moving_example++;
	
        SDL_Delay( 500 );

        /*if (!should_continue) {
            break;
        }*/
    }

	printf ("The program has finished.\n");
    
    //Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();
    
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

bool pop(uint16_t *pc_in) {
    //printf( "value from stack: %d", stack[sp-1] );
    *pc_in = stack[sp - 1];
    //printf( "new instruction value: %d", *instruction );
    sp--; // sp should never be 0 when this is called.. 
    printf ("this is my sp: %d", sp); 
    if (sp <= 0) {
        return false;
    }
    return true;
}

void push(uint16_t pc_in) {
    //printf ("sp: %d\n", sp);
    stack[sp] = pc_in;
    //printf ("instruction: %d\n", instruction);
    sp++;
}


// going to have to decode instructions based on each byte of information

errno_t execute_instruction (uint16_t instruction) {
    // switch statements or function pointers?
    // decision: because of the variable length nature of the operands,
    // switch statements will be more practical.
    
    if (instruction == 0x00e0) { // CLS: Clear the display
        // stuff
    }
    
    switch ( instruction & 0xF000 ) { // first level
        case 0x1000: // JP: Jump to location nnn
            pc = instruction & ~(0xF000); // only preserve nnn
            break;
        case 0x2000: // CALL: Call subroutine at nnn
            sp++;
            push(pc);
            pc = instruction & ~(0xF000);
            break;
        case 0x3000: // SE Vx, byte
            // If Vx == kk, inc pc by 2
            if ( V[(instruction & 0x0F00)] == (instruction & 0x00FF )) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x4000: // SNE Vx, byte
            // Skip next instruction if Vx != kk
            if ( V[(instruction & 0x0F00)] != (instruction & 0x00FF )) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x6000: // LD Vx, byte
            // Set Vx = kk
            // dont worry, no need to cast! upper byte will be discarded safely ;)
            V[(instruction & 0x0F00)] = (instruction & 0x00FF);
        case 0x7000: // ADD Vx, byte
            // Set Vx = Vx + kk
            V[(instruction & 0x0F00)] = V[(instruction & 0x0F00)] + (instruction & 0x00FF);
        default:
            printf("Invalid instruction: %x", instruction);
            break;
    }
    
    /* skipped instructions:
     5xy0
    */
    return 0;
}

int test() {
    printf ("testing stack...");
    pc = 0x0;
    return 0;
}

int initSDL() {
	//Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	}
	else
	{
		//Create window
		//window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        //Create window w/ renderer
        
        SDL_CreateWindowAndRenderer(SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE, 0, &window, &renderer);
        SDL_RenderSetScale(renderer, WINDOW_SCALE, WINDOW_SCALE);
    }
		
	return 0;
}

void draw_sdl_panel() {
    // set panel to black
    // get list of points that need to be drawn
    // draw points and update panel
    // not sure how fast to draw or if it matters?
    
    // decrement the timers through a thread? (this adds more complexity to the application)
    // decision: let the emulator run as fast as possible at first, and then later come back to threading
    return;
}
