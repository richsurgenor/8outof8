//
//  chip8.h
//  8outof8
//
//  Created by Rich Surgenor on 9/2/18.
//  Copyright © 2018 Rich Surgenor. All rights reserved.
//

#include "stdbool.h"
#include "stdlib.h"

#ifndef chip8_h
#define chip8_h

errno_t run(char* rom);
int initSDL();
void draw_SDL_panel();
void set_pc(uint16_t val);
void inc_pc(uint16_t inc);
bool load_rom(const char* rom);
void fetch();
errno_t execute_instruction(uint16_t instruction);

void run_basic_tests(char* rom);

int rand_lim(int limit);

#endif /* chip8_h */
