#ifndef _CHIP_8_
#define _CHIP_8_

#include <stdint.h>

#define MAX_COL 8

#define START_ADDRESS 0x200

#define FONTSET_START_ADDRESS 0x50
#define FONTSET_SIZE 80 // 16 chars, 5 bytes each

#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32


typedef struct {
    uint8_t registers[16]; // V0 - VF
    uint8_t memory[4096];
    uint16_t index_reg;
    uint16_t pc;
    uint16_t stack[16]; // stores 16 program counters
    uint8_t sp;         // 0-16
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keypad[16];
    uint32_t video[64 * 32];
    uint16_t opcode;
    unsigned int rng_state;
} chip8_t;

extern uint8_t font_set[FONTSET_SIZE];

void chip8_init(chip8_t *self);
void chip8_load_rom(const char *path, chip8_t *self);
uint8_t chip8_random_byte(chip8_t *self);
void chip8_cycle(chip8_t *self);

// TODO:
// 1. random byte generator

#endif
