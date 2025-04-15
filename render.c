#include "render.h"
#include "minos.h"

#include <SDL.h>
#include <SDL_ttf.h>

const char* lineNames[21] = {
    "Noway", "Single", "Double", "Triple", "Quad",
    "Penta", "Hexa", "Hepta", "Octa", "Nona", "Deca",
    "Hendeca", "Dodeca", "Trideca", "Tetradeca", "Pentadeca",
    "Hexadeca", "Heptadeca", "Octadeca", "Enneadeca", "Icosa"
};

static int renderFont(SDL_Renderer* renderer, TTF_Font* font, Camera* cam, const char* text, int x, int y, SDL_Color* color, _Bool alignRight) {
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, *color);
    if (!surface) {
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return 1;
    }

    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);

    SDL_Rect rect = {(alignRight ? x - surface->w : x) - cam->x, y - cam->y, surface->w, surface->h};

    if (SDL_RenderCopy(renderer, texture, NULL, &rect) != 0) {
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        return 1;
    }

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return 0;
}

static int renderRect(SDL_Renderer* renderer, SDL_Rect* rect, Camera* cam, int thickness) {
    int x = rect->x - cam->x;
    int y = rect->y - cam->y;

    if (thickness > 0) {
        for (int i = 0; i < thickness; ++i) {
            SDL_Rect border = {x + i, y + i, rect->w - 2 * i, rect->h - 2 * i};
            if (SDL_RenderDrawRect(renderer, &border) != 0) {
                return 1;
            }
        }
    } else {
        SDL_Rect newrect = {x, y, rect->w, rect->h};
        if (SDL_RenderDrawRect(renderer, &newrect) != 0) {
            return 1;
        }
    }

    return 0;
}

static int renderFillRect(SDL_Renderer* renderer, SDL_Rect* rect, Camera* cam) {
    int x = rect->x - cam->x;
    int y = rect->y - cam->y;

    SDL_Rect newrect = {x, y, rect->w, rect->h};
    if (SDL_RenderFillRect(renderer, &newrect) != 0) {
        return 1;
    }

    return 0;
}

static const char* getLineChar(int line) {
    if (line > 0 && line < (int)(sizeof(lineNames) / sizeof(lineNames[0]))) {
        return lineNames[line];
    }
    return lineNames[0];
}