#ifndef RENDER_H
#define RENDER_H

typedef struct Camera {
    int x, y;
} Camera;
Camera cam = {0, 0};

int renderFont(SDL_Renderer* renderer, TTF_Font* font, Camera* cam, const char* text, int x, int y, SDL_Color* color, _Bool alignRight);
int renderRect(SDL_Renderer* renderer, SDL_Rect* rect, Camera* cam, int thickness);
int renderFillRect(SDL_Renderer* renderer, SDL_Rect* rect, Camera* cam);
const char* getLineChar(int line);
int drawViewMino(SDL_Renderer* renderer, Camera* cam, int mino, int x, int y, _Bool forcegray);

extern const char* lineNames[21];

#endif