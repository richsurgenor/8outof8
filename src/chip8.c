#include "chip8.h"
#include "SDL2/SDL.h"
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include "assert.h"

//Screen dimension constants
const int WINDOW_SCALE = 20;
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


#define FONTSET_ADDRESS 0x00
#define FONTSET_BYTES_PER_CHAR 5
#define MAX_SPRITE_SIZE 15

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Relevant to CPU

// Only one thread so static initialization should be okay.
static uint16_t pc;
static uint16_t stack[16];
static uint8_t  sp;

static uint8_t V[16];
static uint16_t I;
static uint16_t delay_timer;
static uint16_t snd_timer;

static uint8_t gfx[64][32];
static bool draw = false;

static char key[16];

// Coordinate system:
// -------------------------
// | (0,0)        (63,0)   |
// |                       |
// | (0,31)       (63,31)  |
// -------------------------



//memory should be an 0xffff array? o-o


errno_t run(char* rom) {
    
    errno_t ret = EXIT_SUCCESS;
    
    //load_rom("wipeoff.rom");
    load_rom(rom);
    
	initSDL();
    
    pc = 0x200;

    for (int i = 0; i < 80; i++) {
        ram[FONTSET_ADDRESS + i] = chip8_fontset[i];
    }
    
    int moving_example = 0;
    bool quit = false;
    
    // game loop
    while ( !quit ) {
        
        // event loop
        while( SDL_PollEvent( &e ) != 0 ) {
            //User requests quit or presses/releases key
            if( e.type == SDL_QUIT ) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_1: key[0x1] = 1; break;
                    case SDLK_2: key[0x2] = 1; break;
                    case SDLK_3: key[0x3] = 1; break;
                    case SDLK_4: key[0xC] = 1; break;
                    case SDLK_q: key[0x4] = 1; break;
                    case SDLK_w: key[0x5] = 1; break;
                    case SDLK_e: key[0x6] = 1; break;
                    case SDLK_r: key[0xD] = 1; break;
                    case SDLK_a: key[0x7] = 1; break;
                    case SDLK_s: key[0x8] = 1; break;
                    case SDLK_d: key[0x9] = 1; break;
                    case SDLK_f: key[0xE] = 1; break;
                    case SDLK_z: key[0xA] = 1; break;
                    case SDLK_x: key[0x0] = 1; break;
                    case SDLK_c: key[0xB] = 1; break;
                    case SDLK_v: key[0xF] = 1; break;
                    case SDLK_ESCAPE: exit(1); break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym) {
                    case SDLK_1: key[0x1] = 0; break;
                    case SDLK_2: key[0x2] = 0; break;
                    case SDLK_3: key[0x3] = 0; break;
                    case SDLK_4: key[0xC] = 0; break;
                    case SDLK_q: key[0x4] = 0; break;
                    case SDLK_w: key[0x5] = 0; break;
                    case SDLK_e: key[0x6] = 0; break;
                    case SDLK_r: key[0xD] = 0; break;
                    case SDLK_a: key[0x7] = 0; break;
                    case SDLK_s: key[0x8] = 0; break;
                    case SDLK_d: key[0x9] = 0; break;
                    case SDLK_f: key[0xE] = 0; break;
                    case SDLK_z: key[0xA] = 0; break;
                    case SDLK_x: key[0x0] = 0; break;
                    case SDLK_c: key[0xB] = 0; break;
                    case SDLK_v: key[0xF] = 0; break;
				}
            }
        }
            
        //bool should_continue = pop(&opcode);
        
        if( window == NULL || renderer == NULL)
		{
			printf( "Window or renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
            exit(1);
		}
		
        // draw = true
        if (draw) {
			//Get window surface
			//screenSurface = SDL_GetWindowSurface( window );

			//Fill the surface white
			//SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0x00, 0x00, 0x00 ) );
			
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            
            SDL_RenderClear(renderer); // sets background color
            
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
            
            //gfx[0][0] = 1;
            //gfx[63][31] = 1;
            
            SDL_Point points[SCREEN_WIDTH * SCREEN_HEIGHT]; // 2048 points max
            int actual_pts = 0;
            
            for (int i = 0; i < SCREEN_WIDTH; i++) {
                for (int j = 0; j < SCREEN_HEIGHT; j++) {
                    if (gfx[i][j]) { // if point is set to 1
                        points[actual_pts].x = i;
                        points[actual_pts].y = j;
                        actual_pts++;
                    }
                }
            }
                
            //SDL_Point test_point = { .x = 8, .y=8 };
            
            /*for (int i = 0; i < 14; i++) {
                points[i].x = i+moving_example;
                points[i].y = i+moving_example;
            }
            
            points[14].x = 0;
            points[14].y = 0;
            points[15].x = 63;
            points[15].y = 0;
            points[16].x = 0;
            points[16].y = 31; */
           // points[17].x = 0;
           // points[17].y = 0;
            
            //const SDL_Point* point_array = { points };
            
            SDL_RenderDrawPoints( renderer, points, actual_pts); // note this includes 0
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
            draw = false; // wait to be toggled again.
		}
        
        fetch();
        
        if (delay_timer > 0) {
            delay_timer--;
        }
        
        moving_example++;
	
        SDL_Delay( 1 );

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

void fetch() {
    uint16_t instruction;
    // retrieve next instruction
    instruction = ram[pc+0];
    instruction = instruction << 8;
    instruction |= ram[pc+1];
    
    execute_instruction(instruction);
    
}

bool load_rom(const char* rom) {
    
    char *buffer;
    long filelen;
    
    FILE* f_rom = fopen(rom, "r+b");
    
    if (!f_rom) {
        exit(1); // opening file failed owo
    }
    
    fseek(f_rom, 0, SEEK_END);
    filelen = ftell(f_rom);
    rewind(f_rom);
    
    buffer = (char *) malloc( (filelen + 1) * sizeof(char) );
    fread(buffer, filelen, 1, f_rom);
    fclose(f_rom);
    
    uint16_t start = 0x200;
    for (int i = 0; i < filelen; i++) {
        ram[start+i] = *(buffer+i);
    }
    
    //printf("first slot: %x", ram[0x200]);
    //printf("file length: %d", (int) filelen);
    //printf("last slot: %x", ram[0x2cd]);
   
    return true;
}

void set_pc(uint16_t val) {
    pc = val;
}

void inc_pc(uint16_t inc) {
    pc += 2 * inc;
}

bool is_key_pressed(uint16_t* pressed_key) {
    for (int i = 0; i < 16; i++) {
        if (key[i]) {
            *pressed_key = i;
            return true;
        }
    }
    return false;
}

// going to have to decode instructions based on each byte of information

errno_t execute_instruction (uint16_t instruction) {
    // switch statements or function pointers?
    // decision: because of the variable length nature of the operands,
    // switch statements will be more practical.
    
    uint8_t nibbles[4];
    nibbles[0] = (instruction & 0x000F);
    nibbles[1] = (instruction & 0x00F0) >> 4;
    nibbles[2] = (instruction & 0x0F00) >> 8;
    nibbles[3] = (instruction & 0xF000) >> 12;
    
    uint8_t kk = nibbles[0] | ( nibbles[1] << 4 );
    uint16_t nnn = kk | (nibbles[2] << 8); // first 3 nibbles (from right)
    
    // note that instructions are in multiples of 2, so when spec says inc pc by 2, it will be inc by 4 and so on.
    switch ( instruction & 0xF000 ) { // first level
        case 0x0000: {
            switch(instruction) {
                //case 0x0000:
                //    inc_pc(1);
                //    break;
                case 0x00e0:
                    memset(&gfx, 0x0, sizeof(gfx));
                    draw = true;
                    inc_pc(1);
                    break;
                case 0x00ee:
                    set_pc(stack[--sp]);
                    break;
                default: // assume no op
                    //printf("unknown instruc: %x", instruction);
                    //inc_pc(1);
                    break;
            }
            break;
        }
        case 0x1000: // JP: Jump to location nnn
            set_pc(nnn); // instruction & ~(0xF000); // only preserve nnn
            break;
        case 0x2000: // CALL: Call subroutine at nnn
            stack[sp++] = pc + 2;
            set_pc(nnn); // instruction & ~(0xF000);
            break;
        case 0x3000: // SE Vx, byte
            // If Vx == kk, inc pc by 2
            if ( V[ nibbles[2] ] == kk) {
                inc_pc(2);
            } else {
                inc_pc(1);
            }
            break;
        case 0x4000: // SNE Vx, byte
            // Skip next instruction if Vx != kk
            if ( V[ nibbles[2] ] != ( kk )) {
                inc_pc(2);
            } else {
                inc_pc(1);
            }
            break;
        case 0x5000:
            if ( V[ nibbles[2] ] == ( kk )) {
                inc_pc(2);
            } else {
                inc_pc(1);
            }
            break;
        case 0x6000: // LD Vx, byte
            // Set Vx = kk
            V[ nibbles[2] ] = ( kk );
            inc_pc(1);
            break;
        case 0x7000: // 7xkk : ADD Vx, byte
            // Set Vx = Vx + kk
            V[ nibbles[2] ] = V[ nibbles[2] ] + kk;
            inc_pc(1);
            break;
        case 0x8000: {
            switch( instruction & 0xF00F) {
                case 0x8000: // 8xy0 : LD Vx, Vy
                    // Set Vx = Vy
                    V[ nibbles[2] ] = V[ nibbles[1] ];
                    inc_pc(1);
                    break;
                case 0x8001: // 8xy1 : OR Vx, Vy
                    // Set Vx = Vx OR Vy
                    V[ nibbles[2] ] = V[ nibbles[2] ] | V [ nibbles[1] ];
                    inc_pc(1);
                    break;
                case 0x8002: // 8xy2 : AND Vx, Vy
                    V[ nibbles[2] ] = V[ nibbles[2] ] & V [ nibbles[1] ];
                    inc_pc(1);
                    break;
                case 0x8003: // 8xy3 : XOR Vx, Vy
                    V[ nibbles[2] ] = V[ nibbles[2] ] ^ V [ nibbles[1] ];
                    inc_pc(1);
                    break;
                case 0x8004: // 8xy4 : ADD Vx, Vy
                    V[ nibbles[2] ] = V[ nibbles[2] ] + V [ nibbles[1] ];
                    inc_pc(1);
                    break;
                case 0x8005: // 8xy5 : SUB Vx, Vy
                    V[ nibbles[2] ] = V[ nibbles[2] ] - V [ nibbles[1] ];
                    inc_pc(1);
                    break;
                case 0x8006: // 8xy6 : SHR Vx {, Vy}
                    // Set Vx = Vx SHR 1.
                    if ( V[ nibbles[2] ] & 0x1) { // if LSBit of Vx=1
                        V[0xF] = 1;
                    } else {
                        V[0xf] = 0;
                    }
                    inc_pc(1);
                    break;
                case 0x8007: // 8xy7 : SUBN Vx, Vy
                    // Set Vx = Vy - Vx, set VF = NOT borrow
                    if ( V[ nibbles[1] ] > V[ nibbles[2] ] ) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    
                    V[ nibbles[2] ] = V[ nibbles[1] ] - V[ nibbles[2] ];
                    inc_pc(1);
                    break;
                case 0x800E: // SHL Vx {, Vy}
                    // Set Vx = Vx SHL 1
                    if ( V[ nibbles[2] ] & 8000) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    inc_pc(1);
                    break;
                default:
                    printf("Invalid instruction: %x", instruction);
                    break;
            }
            break;
        }
        case 0x9000: // SNE Vx, Vy
            if ( V[ nibbles[2] ] != V[ nibbles[1] ]) {
                inc_pc(2);
            } else {
                inc_pc(1);
            }
            break;
        case 0xA000: // LD I, addr
            // Set I = nnn
            I = nnn;
            inc_pc(1);
            break;
        case 0xB000: // JP V0, addr
            // Jump to location nnn + V0
            set_pc( V[0] + nnn );
            break;
        case 0xC000: // RND Vx, byte
            // Set Vx = random byte AND kk
            V[ nibbles[2] ] = rand_lim(0xFF) & kk;
            inc_pc(1);
            break;
        case 0xD000: { // DRW Vx, Vy, nibble
            // Display n-byte sprite starting at memory location I
            // at (Vx, Vy), set VF = collision
            
            assert(nibbles[0] <= MAX_SPRITE_SIZE);
            
            V[0xF] = 0;
            
            // TODO: add mod for screen wrap around
            for (int i = 0; i < nibbles[0]; i++) {
                uint8_t sprite = ram[I+i];
                for (int j = 0; j < 8; j++) {
                    uint8_t mask = 1 << j;
                    uint8_t *pixel = &gfx[ ( ( V[ nibbles[2]]) + j ) % SCREEN_WIDTH ] [ ( V[ nibbles[1]] + i ) % SCREEN_HEIGHT ] ;
                    uint8_t new_pixel = (sprite & mask) >> j;
                    
                    if ( (*pixel == 1 && new_pixel == 1) ) {
                        // if there isn't already a collision, and the (x,y) and the
                        // current bit of sprite are both 1, then there will be a collision.
                        V[0xf] = 1;
                    }
                    *pixel = *pixel ^ new_pixel;
                    //00-ff
                    // get each bit in the byte that comes in
                }
            }
            
            inc_pc(1);
            draw = true;
            break;
        }
        case 0xE000: {
            switch( instruction & 0xF0FF) {
                case 0xE09E:
                    if ( key[ V[ nibbles[2] ] ] ) {
                        inc_pc(2);
                    } else {
                        inc_pc(1);
                    }
                    break;
                case 0xE0A1:
                    if ( !key[ V[ nibbles[2] ] ] ) {
                        inc_pc(2);
                    } else {
                        inc_pc(1);
                    }
                    break;
                default:
                    printf("Invalid instruction: %x", instruction);
                    break;
            }
            break;
        }
        case 0xF000: {
            switch( instruction & 0xF0FF ) {
                case 0xF007:
                    V[ nibbles[ 2] ] = delay_timer;
                    inc_pc(1);
                    break;
                case 0xF00A: { // LD Vx, K
                    // Stop all execution until key is pressed
                    // and set Vx to key.
                    
                    // Note that this will cause cpu to loop until a
                    // key is pressed.
                    uint16_t pressed_key;
                    if (is_key_pressed(&pressed_key)) {
                        V[ nibbles[2] ] = pressed_key;
                        inc_pc(1);
                    }
                    break;
                }
                case 0xF015:
                    delay_timer = V[ nibbles[2] ];
                    inc_pc(1);
                    break;
                case 0xF018:
                    snd_timer = V[ nibbles[2] ];
                    printf("BEEEEEEEP\N");
                    inc_pc(1);
                    break;
                case 0xF01E: // ADD I, Vx
                    I = I + V[ nibbles[2] ];
                    inc_pc(1);
                    break;
                case 0xF029:
                    I = FONTSET_BYTES_PER_CHAR * V[ nibbles[2] ];
                    inc_pc(1);
                    break;
                case 0xF033:
                    ram[I] = V[ nibbles[2] ] / 100;
                    ram[I+1] = (V[ nibbles[2] ] / 10) % 10;
                    ram[I+2] = V[ nibbles[2] ] % 10;
                    inc_pc(1);
                    break;
                case 0xF055: { //TODO: FIGURE OUT WHY BROKEN.
                    for ( int i = 0; i <= V[ nibbles[2] ]; i++ ) {
                        ram[I+i] = V[i];
                    }
                    // not clear
                    I += V[ nibbles[2] ] + 1;
                    inc_pc(1);
                    break;
                }
                case 0xF065:
                    for ( int i = 0; i <= V[ nibbles[2] ]; i++ ) {
                        V[i] = ram[I+i];
                    }
                    // not clear
                    I += V[ nibbles[2] ] + 1;
                    inc_pc(1);
                    break;
                default:
                    printf("Invalid instruction: %x", instruction);
                    break;
            }
            break;
        }
            
        default:
            printf("Invalid instruction: %x", instruction);
            break;
    }
    
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


void run_basic_tests(char* rom) {
    load_rom(rom);
    
}

int rand_lim(int limit) {
    /* return a random number between 0 and limit inclusive.
     */
    
    int divisor = RAND_MAX/(limit+1);
    int retval;
    
    do {
        retval = rand() / divisor;
    } while (retval > limit);
    
    return retval;
}

