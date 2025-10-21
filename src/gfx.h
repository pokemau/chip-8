#ifndef _GFX_H_
#define _GFX_H_

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} gfx_t;

void gfx_init(gfx_t *platform, const char *title, int window_width,
              int window_height, int texture_width, int texture_height);
void gfx_destroy(gfx_t *platform);
void gfx_update(gfx_t *platform, const void *buffer, int pitch);
bool gfx_proc_input(gfx_t *platform, uint8_t *keys);

#endif
