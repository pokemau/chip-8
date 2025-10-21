#include "chip8.h"
#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Resources
// https://austinmorlan.com/posts/chip8_emulator
// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Scale> <Delay> <ROM>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int video_scale = atoi(argv[1]);
    int cycle_delay = atoi(argv[2]);
    const char *rom_filename = argv[3];

    gfx_t platform;
    gfx_init(&platform, "CHIP-8 Emulator", VIDEO_WIDTH * video_scale,
             VIDEO_HEIGHT * video_scale, VIDEO_WIDTH, VIDEO_HEIGHT);

    chip8_t chip8;
    chip8_init(&chip8);
    chip8_load_rom(rom_filename, &chip8);

    int video_pitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    clock_t last_cycle_time = clock();
    bool quit = false;

    while (!quit) {
        quit = gfx_proc_input(&platform, chip8.keypad);

        clock_t current_time = clock();
        float dt = ((float)(current_time - last_cycle_time) / CLOCKS_PER_SEC) *
                   1000.0f;

        if (dt > cycle_delay) {
            last_cycle_time = current_time;

            chip8_cycle(&chip8);

            gfx_update(&platform, chip8.video, video_pitch);
        }
    }

    gfx_destroy(&platform);

    return 0;
}
