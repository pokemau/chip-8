#include "chip8.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

uint8_t font_set[FONTSET_SIZE] = {
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

void chip8_init(chip8_t *self) {
    memset(self, 0, sizeof(chip8_t));

    self->pc = START_ADDRESS;
    self->rng_state = (unsigned int)time(NULL);

    for (unsigned int i = 0; i < FONTSET_SIZE; i++) {
        self->memory[FONTSET_START_ADDRESS + i] = font_set[i];
    }
}

uint8_t chip8_random_byte(chip8_t *self) {
    self->rng_state = self->rng_state * 1103515245 + 12345;
    return (self->rng_state >> 16) & 0xFF;
}

void chip8_load_rom(const char *path, chip8_t *self) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "failed to open %s\n", path);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    int len = ftell(file);
    char *buffer = malloc(len);

    rewind(file);
    fread(buffer, 1, len, file);
    fclose(file);

    for (long i = 0; i < len; i++) {
        self->memory[START_ADDRESS + i] = buffer[i];
    }
    free(buffer);
}

void chip8_op_00e0(chip8_t *self) {
    memset(self->video, 0, sizeof(self->video));
}

void chip8_op_00ee(chip8_t *self) {
    --self->sp;
    self->pc = self->stack[self->sp];
}

void chip8_op_1nnn(chip8_t *self) {
    uint16_t address = self->opcode & 0x0FFF;
    self->pc = address;
}

void chip8_op_2nnn(chip8_t *self) {
    uint16_t address = self->opcode & 0x0FFF;
    self->stack[self->sp++] = self->pc;
    self->pc = address;
}

void chip8_op_3xkk(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t byte = self->opcode & 0x00FF;
    if (self->registers[Vx] == byte) {
        self->pc += 2;
    }
}

void chip8_op_4xkk(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t byte = self->opcode & 0x00FF;

    if (self->registers[Vx] != byte) {
        self->pc += 2;
    }
}

void chip8_op_5xy0(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;

    if (self->registers[Vx] == self->registers[Vy]) {
        self->pc += 2;
    }
}

void chip8_op_6xkk(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t byte = self->opcode & 0x00FF;
    self->registers[Vx] = byte;
}

void chip8_op_7xkk(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t byte = self->opcode & 0x00FF;
    self->registers[Vx] += byte;
}

void chip8_op_8xy0(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;
    self->registers[Vx] = self->registers[Vy];
}

void chip8_op_8xy1(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;
    self->registers[Vx] |= self->registers[Vy];
}

void chip8_op_8xy2(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;
    self->registers[Vx] &= self->registers[Vy];
}

void chip8_op_8xy3(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;
    self->registers[Vx] ^= self->registers[Vy];
}

void chip8_op_8xy4(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;

    uint16_t sum = self->registers[Vx] + self->registers[Vy];
    if (sum > 255) {
        self->registers[0xF] = 1;
    } else {
        self->registers[0xF] = 0;
    }

    self->registers[Vx] = sum & 0xFF;
}

void chip8_op_8xy5(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;

    if (self->registers[Vx] > self->registers[Vy]) {
        self->registers[0xF] = 1;
    } else {
        self->registers[0xF] = 0;
    }
    self->registers[Vx] -= self->registers[Vy];
}

void chip8_op_8xy6(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    self->registers[0xF] = (self->registers[Vx] & 0x1);
    self->registers[Vx] >>= 1;
}

void chip8_op_8xy7(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;

    if (self->registers[Vy] > self->registers[Vx]) {
        self->registers[0xF] = 1;
    } else {
        self->registers[0xF] = 0;
    }

    self->registers[Vx] = self->registers[Vy] - self->registers[Vx];
}

void chip8_op_8xye(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;

    self->registers[0xF] = (self->registers[Vx] & 0x80) >> 7;

    self->registers[Vx] <<= 1;
}

void chip8_op_9xy0(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;

    if (self->registers[Vx] != self->registers[Vy]) {
        self->pc += 2;
    }
}

void chip8_op_Annn(chip8_t *self) {
    uint16_t address = self->opcode & 0x0FFF;
    self->index_reg = address;
}

void chip8_op_Bnnn(chip8_t *self) {
    uint16_t address = self->opcode & 0x0FFF;
    self->pc = self->registers[0] + address;
}

void chip8_op_Cxkk(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t byte = self->opcode & 0x00FF;

    self->registers[Vx] = chip8_random_byte(self) & byte;
}

void chip8_op_Dxyn(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t Vy = (self->opcode & 0x00F0) >> 4;
    uint8_t height = self->opcode & 0x000F;

    uint8_t x_pos = self->registers[Vx] % VIDEO_WIDTH;
    uint8_t y_pos = self->registers[Vy] % VIDEO_HEIGHT;

    self->registers[0xF] = 0;

    for (unsigned int row = 0; row < height; row++) {
        uint8_t byte = self->memory[self->index_reg + row];

        for (unsigned int col = 0; col < MAX_COL; col++) {
            uint8_t sprite_pixel = byte & (0x80 >> col);

            uint32_t *screen_pixel =
                &self->video[(y_pos + row) * VIDEO_WIDTH + (x_pos + col)];

            if (sprite_pixel) {
                if (*screen_pixel == 0xFFFFFFFF) {
                    self->registers[0xF] = 1;
                }
                *screen_pixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void chip8_op_Ex9E(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t key = self->registers[Vx];

    if (self->keypad[key]) {
        self->pc += 2;
    }
}

void chip8_op_ExA1(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t key = self->registers[Vx];

    if (!self->keypad[key]) {
        self->pc += 2;
    }
}

void chip8_op_Fx07(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    self->registers[Vx] = self->delay_timer;
}

void chip8_op_Fx0A(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;

    uint8_t key_pressed = 0;

    for (uint8_t i = 0; i < 16; i++) {
        if (self->keypad[i]) {
            self->registers[Vx] = i;
            key_pressed = i;
            break;
        }
    }
    if (!key_pressed) {
        self->pc -= 2;
    }
}

void chip8_op_Fx15(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    self->delay_timer = self->registers[Vx];
}

void chip8_op_Fx18(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    self->sound_timer = self->registers[Vx];
}

void chip8_op_Fx1E(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    self->index_reg += self->registers[Vx];
}

void chip8_op_Fx29(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t digit = self->registers[Vx];

    self->index_reg = FONTSET_START_ADDRESS + (5 * digit);
}

void chip8_op_Fx33(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;
    uint8_t val = self->registers[Vx];

    // ones
    self->memory[self->index_reg + 2] = val % 10;
    val /= 10;

    // tens
    self->memory[self->index_reg + 1] = val % 10;
    val /= 10;

    // hundreds
    self->memory[self->index_reg] = val % 10;
}

void chip8_op_Fx55(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;

    for (uint8_t i = 0; i <= Vx; i++) {
        self->memory[self->index_reg + i] = self->registers[i];
    }
}

void chip8_op_Fx65(chip8_t *self) {
    uint8_t Vx = (self->opcode & 0x0F00) >> 8;

    for (uint8_t i = 0; i <= Vx; i++) {
        self->registers[i] = self->memory[self->index_reg + i];
    }
}

void chip8_cycle(chip8_t *self) {
    self->opcode = (self->memory[self->pc] << 8) | self->memory[self->pc + 1];

    self->pc += 2;

    uint16_t first_nibble = (self->opcode & 0xF000) >> 12;

    switch (first_nibble) {
    case 0x0:
        switch (self->opcode & 0x00FF) {
        case 0xE0:
            chip8_op_00e0(self);
            break;
        case 0xEE:
            chip8_op_00ee(self);
            break;
        }
        break;
    case 0x1:
        chip8_op_1nnn(self);
        break;
    case 0x2:
        chip8_op_2nnn(self);
        break;
    case 0x3:
        chip8_op_3xkk(self);
        break;
    case 0x4:
        chip8_op_4xkk(self);
        break;
    case 0x5:
        chip8_op_5xy0(self);
        break;
    case 0x6:
        chip8_op_6xkk(self);
        break;
    case 0x7:
        chip8_op_7xkk(self);
        break;
    case 0x8:
        switch (self->opcode & 0x000F) {
        case 0x0:
            chip8_op_8xy0(self);
            break;
        case 0x1:
            chip8_op_8xy1(self);
            break;
        case 0x2:
            chip8_op_8xy2(self);
            break;
        case 0x3:
            chip8_op_8xy3(self);
            break;
        case 0x4:
            chip8_op_8xy4(self);
            break;
        case 0x5:
            chip8_op_8xy5(self);
            break;
        case 0x6:
            chip8_op_8xy6(self);
            break;
        case 0x7:
            chip8_op_8xy7(self);
            break;
        case 0xE:
            chip8_op_8xye(self);
            break;
        }
        break;
    case 0x9:
        chip8_op_9xy0(self);
        break;
    case 0xA:
        chip8_op_Annn(self);
        break;
    case 0xB:
        chip8_op_Bnnn(self);
        break;
    case 0xC:
        chip8_op_Cxkk(self);
        break;
    case 0xD:
        chip8_op_Dxyn(self);
        break;
    case 0xE:
        switch (self->opcode & 0x00FF) {
        case 0x9E:
            chip8_op_Ex9E(self);
            break;
        case 0xA1:
            chip8_op_ExA1(self);
            break;
        }
        break;
    case 0xF:
        switch (self->opcode & 0x00FF) {
        case 0x07:
            chip8_op_Fx07(self);
            break;
        case 0x0A:
            chip8_op_Fx0A(self);
            break;
        case 0x15:
            chip8_op_Fx15(self);
            break;
        case 0x18:
            chip8_op_Fx18(self);
            break;
        case 0x1E:
            chip8_op_Fx1E(self);
            break;
        case 0x29:
            chip8_op_Fx29(self);
            break;
        case 0x33:
            chip8_op_Fx33(self);
            break;
        case 0x55:
            chip8_op_Fx55(self);
            break;
        case 0x65:
            chip8_op_Fx65(self);
            break;
        }
        break;
    }

    if (self->delay_timer > 0) {
        --self->delay_timer;
    }

    if (self->sound_timer > 0) {
        --self->sound_timer;
    }
}
