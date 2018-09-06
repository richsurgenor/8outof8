//
//  main.c
//  8outof8
//
//  Created by Rich Surgenor on 9/2/18.
//  Copyright © 2018 Rich Surgenor. All rights reserved.
//

#include <stdio.h>
#include "chip8.h"

#define ENABLE_BASIC_TESTS 0

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./play <game>\n");
        exit(2);
    }
#if ENABLE_BASIC_TESTS == 1
    run_basic_tests(argv[1]);
#else
    run(argv[1]);
#endif
}
