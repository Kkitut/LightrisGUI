#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#include "lightris.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define CANVAS_WIDTH 16*16 // 256
#define CANVAS_HEIGHT 9*16 // 144

const SDL_Color cWhite = {255, 255, 255, 255};
const SDL_Color cRed = {255, 150, 150, 255};
const SDL_Color cYellow = {255, 255, 150, 255};
const SDL_Color cGreen = {150, 255, 150, 255};
const SDL_Color cBlack = {0, 0, 0, 255};

const SDL_Color cgGrid = {80, 80, 80, 0};

const SDL_Color cmI = {117, 255, 255, 255};
const SDL_Color cmJ = {117, 126, 255, 255};
const SDL_Color cmL = {255, 186, 117, 255};
const SDL_Color cmS = {117, 255, 133, 255};
const SDL_Color cmT = {255, 117, 248, 255};
const SDL_Color cmZ = {255, 117, 117, 255};
const SDL_Color cmO = {255, 250, 117, 255};
const SDL_Color cmG = {125, 125, 125, 255};
const SDL_Color cmB = {36, 36, 36, 255};

#define SetDrawColor(renderer, color) SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a)

SDL_Color ColorByMino(int colorIndex) {
    switch (colorIndex) {
        case 1: return cmI;
        case 2: return cmJ;
        case 3: return cmL;
        case 4: return cmS;
        case 5: return cmT;
        case 6: return cmZ;
        case 7: return cmO;
        case 8: return cmG;
        case 9: return cmB;
        default: return cWhite;
    }
}

static int drawViewMino(SDL_Renderer* renderer, Camera* cam, int mino, int x, int y, _Bool forcegray) {
    if (forcegray) {
        SetDrawColor(renderer, ColorByMino(8));
    } else {
        SetDrawColor(renderer, ColorByMino(mino));
    }

    SDL_Rect rect;
    switch (mino) {
        case 1:
            rect = (SDL_Rect) {x-3, y+3, 24, 6};
            renderFillRect(renderer, &rect, cam);
            break;
        case 2:
            rect = (SDL_Rect) {x, y, 6, 6};
            renderFillRect(renderer, &rect, cam);
            rect = (SDL_Rect) {x, y+6, 18, 6};
            renderFillRect(renderer, &rect, cam);
            break;
        case 3:
            rect = (SDL_Rect) {x+12, y, 6, 6};
            renderFillRect(renderer, &rect, cam);
            rect = (SDL_Rect) {x, y+6, 18, 6};
            renderFillRect(renderer, &rect, cam);
            break;
        case 4:
            rect = (SDL_Rect) {x+6, y, 12, 6};
            renderFillRect(renderer, &rect, cam);
            rect = (SDL_Rect) {x, y+6, 12, 6};
            renderFillRect(renderer, &rect, cam);
            break;
        case 5:
            rect = (SDL_Rect) {x+6, y, 6, 6};
            renderFillRect(renderer, &rect, cam);
            rect = (SDL_Rect) {x, y+6, 18, 6};
            renderFillRect(renderer, &rect, cam);
            break;
        case 6:
            rect = (SDL_Rect) {x, y, 12, 6};
            renderFillRect(renderer, &rect, cam);
            rect = (SDL_Rect) {x+6, y+6, 12, 6};
            renderFillRect(renderer, &rect, cam);
            break;
        case 7:
            rect = (SDL_Rect) {x+3, y, 12, 12};
            renderFillRect(renderer, &rect, cam);
            break;
    }

    return 0;
}


//Handling ARR DAS SDF
//int handling[3] = {30, 200, 30};
int handling[3] = {0, 130, 0};
int handlingclock[5] = {0, 0, 0, 0, 0};
_Bool dasactive[2] = {0, 0};

int forcedrop = 900;
int forcedropTimer = 0;

static void gravityReset() {
    forcedropTimer = clock() + forcedrop;
}

static void drop(int clock) {
    lineClear = 0;
    int downy = y;
    gravityReset();
    for (;;) {
        if (hand == 6) {
            if ((FieldGetBits2(x, downy-1) & 0b1111)) {
                break;
            }
        } else if (hand) {
            if ((FieldGetBits3(x, downy-1) & TETROMINOS[hand][currot])) {
                break;
            }
        } else {
            if ((FieldGetBits4(x, downy-1) & TETROMINOS[0][currot])) {
                break;
            }
        }
        downy--;
    }

    int data = 0;
    FieldSetColor(&data, hand+1);

    if (hand == 6) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                field[downy + i][x + j] = data;
            }
        }
    } else if (hand) {
        int minoindex = 2;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (TETROMINOS[hand][currot] & (1 << (minoindex * 3 + j))) {
                    field[downy + i-1][x + j-1] = data;
                }
            }
            minoindex--;
        }
    } else {
        int minoindex = 3;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (TETROMINOS[0][currot] & (1 << (minoindex * 4 + j))) {
                    field[downy + i-2][x + j-1] = data;
                }
            }
            minoindex--;
        }
    }

    for (int i = 0; i < FIELD_SIZE_Y; i++) {
        _Bool full = 1;
        for (int j = 0; j < FIELD_SIZE_X; j++) {
            if (field[i][j] == 0) {
                full = 0;
                break;
            }
        }
        if (full) {
            for (int k = i; k < FIELD_SIZE_Y - 1; k++) {
                memcpy(field[k], field[k + 1], sizeof(field[k]));
            }
            memset(field[FIELD_SIZE_Y - 1], 0, sizeof(field[0]));
            i--;
            lineClear++;
        }
    }

    if (lineClear) {
        comboCount++;
    } else {
        comboCount = 0;
    }

    bagrotate++;
    if (bagrotate > 7) {
        bagrotate = 1;
        bag[0] = bag[1];
        bag[1] = generateRandomBag();
    }
    isHolding = 0;
    hand = getBagRotated(bag, 0, bagrotate);
    resetPos();
    currot = 0;
}

static _Bool gravity(int clock) {
    if (forcedropTimer < clock) {
        if (isCanMoveAt(0)) {
            gravityReset();
            y--;
        } else {
            drop(clock);
        }
        return 1;
    }
    return 0;
}

static int drawField(SDL_Renderer* renderer) {
    for (int i = 0; i < FIELD_SIZE_Y; i++) {
        for (int j = 0; j < FIELD_SIZE_X; j++) {
            if (!field[i][j]) {
                continue;
            }
            SetDrawColor(renderer, ColorByMino(FieldGetColor(field[i][j])));
            SDL_Rect rect = {97 + 6 * j, 126 - 6 * i, 6, 6};
            renderFillRect(renderer, rect);
        }
    }
    return 0;
}

static int drawMino(SDL_Renderer* renderer, int data, _Bool ghost) {
    if (ghost) {
        SDL_Color color = ColorByMino(FieldGetColor(data));
        color.r -= 60;
        color.g -= 60;
        color.b -= 60;
        SetDrawColor(renderer, color);
    } else {
        SetDrawColor(renderer, ColorByMino(FieldGetColor(data)));
    }
    int bitIndex = 0;
    if (FieldGetColor(data) == 1) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (TETROMINOS[0][FieldGetRot(data)] & (1 << bitIndex)) {
                    SDL_Rect rect = {97 + (getX(data) - 1) * 6 + j * 6 - 6, 126 - (getY(data) - 1) * 6 + i * 6 - 6, 6, 6};
                    renderFillRect(renderer, rect);
                }
                bitIndex++;
            }
        }
    } else if (FieldGetColor(data) == 7) {
        SDL_Rect rect = {97 + (getX(data) - 1) * 6, 126 - (getY(data) - 1) * 6 - 6, 12, 12};
        renderFillRect(renderer, rect);
    } else {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (TETROMINOS[FieldGetColor(data)-1][FieldGetRot(data)] & (1 << bitIndex)) {
                    SDL_Rect rect = {97 + (getX(data) - 1) * 6 + j * 6 - 6, 126 - (getY(data) - 1) * 6 + i * 6 - 6, 6, 6};
                    renderFillRect(renderer, rect);
                }
                bitIndex++;
            }
        }
    }
    return 0;
}


int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_FALSE) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "LightrisGUI",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Setup FAIL",
            "Create Window Failed",
            0
        );
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Renderer FAIL",
            "Create Renderer Failed",
            0
        );
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, CANVAS_WIDTH, CANVAS_HEIGHT);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    TTF_Font* font7 = TTF_OpenFont("font\\Galmuri7.ttf", 8);
    TTF_Font* font9 = TTF_OpenFont("font\\Galmuri9.ttf", 10);
    TTF_Font* font11 = TTF_OpenFont("font\\Galmuri11.ttf", 12);
    if (!font7 || !font9 || !font11) {
        printf("Font Load Error: %s\n", TTF_GetError());
        return 1;
    }

    _Bool nab = 0;
    int menusel = 0;
    _Bool issetting = 0;
    int scn = 0;
    register _Bool draw = 1;

    resetPos();

    srand(time(NULL)+clock());
    bag[0] = generateRandomBag();
    bag[1] = generateRandomBag();
    hand = getBagRotated(bag, 0, bagrotate);

    SDL_Event e;

    clock_t ct = clock();

Scn0:
    for (;;) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                goto ScnEnd;
            }
        }
        ct = clock();
        if (issetting) {

        } else {
            if (getKeyDown(VK_UP)) {
                draw = 1;
                menusel--;
                if (menusel < 1) {
                    menusel = 3;
                }
                nab = 0;
            }

            if (getKeyDown(VK_DOWN)) {
                draw = 1;
                menusel++;
                if (menusel > 3) {
                    menusel = 1;
                }
                nab = 0;
            }

            if (getKeyDown(VK_SPACE) || getKeyDown(VK_RETURN)) {
                draw = 1;
                switch (menusel) {
                    case 1:
                        gravityReset();
                        goto Scn1;
                        break;
                    case 2:
                        nab = 1;
                        break;
                    case 3:
                        goto ScnEnd;
                        break;
                    default:
                        nab = 0;
                        break;
                }
            }

            if (draw) {
                draw = 0;

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect rect = {68, 32, 120, 80};
                renderRect(renderer, rect, 2);
                if (menusel == 1) {
                    renderFont(renderer, font11, "- Run -", 106, 50, &cGreen, 0);
                } else {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    renderFont(renderer, font11, "Run", 117, 50, &cWhite, 0);
                }
                if (menusel == 2) {
                    renderFont(renderer, font11, "- Options -", 93, 66, &cGreen, 0);
                } else {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    renderFont(renderer, font11, "Options", 104, 66, &cWhite, 0);
                }
                if (menusel == 3) {
                    renderFont(renderer, font11, "- Exit -", 105, 82, &cGreen, 0);
                } else {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    renderFont(renderer, font11, "Exit", 116, 82, &cWhite, 0);
                }

                if (nab) {
                    renderFont(renderer, font7, "Not available", 0, -1, &cRed, 0);
                }

                renderFont(renderer, font7, "0.0.0 DEMO", 0, 135, &cWhite, 0);
                SDL_RenderPresent(renderer);
            }
            SDL_Delay(1);
        }
    }

Scn1:
    for (;;) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                goto ScnEnd;
            }
        }
        ct = clock();

        if (getKey('I')) {
            draw = 1;
            cam.y--;
        }
        if (getKey('K')) {
            draw = 1;
            cam.y++;
        }
        if (getKey('J')) {
            draw = 1;
            cam.x--;
        }
        if (getKey('L')) {
            draw = 1;
            cam.x++;
        }

        if (gravity(ct)) {
            draw = 1;
        }

        if (getKey(VK_DOWN)) {
            if (isCanMoveAt(0) && handlingclock[2] < ct) {
                gravityReset();
                handlingclock[2] = handling[2] + ct;
                if (handling[2] <= 0) {
                    while (isCanMoveAt(0)) {
                        y--;
                    }
                } else {
                    y--;
                }
                draw = 1;
            }
        }
        if (getKey(VK_LEFT)) {
            if (isCanMoveAt(1)) {
                if (!dasactive[0]) {
                    dasactive[0] = 1;
                    handlingclock[0] = ct + handling[1];
                    x -= 1;
                } else if (handlingclock[0] < ct && handlingclock[3] < ct) {
                    handlingclock[0] = ct + handling[0];
                    if (handling[0] == 0) {
                        while (isCanMoveAt(1)) x -= 1;
                    } else {
                        x -= 1;
                    }
                }
                draw = 1;
            }
        } else {
            dasactive[0] = 0;
            handlingclock[3] = ct + handling[1];
        }

        if (getKey(VK_RIGHT)) {
            if (isCanMoveAt(2)) {
                if (!dasactive[1]) {
                    dasactive[1] = 1;
                    handlingclock[1] = ct + handling[1];
                    x += 1;
                } else if (handlingclock[1] < ct && handlingclock[4] < ct) {
                    handlingclock[1] = ct + handling[0];
                    if (handling[0] == 0) {
                        while (isCanMoveAt(2)) x += 1;
                    } else {
                        x += 1;
                    }
                }
                draw = 1;
            }
        } else {
            dasactive[1] = 0;
            handlingclock[4] = ct + handling[1];
        }

        if (getKeyDown('Z')) {
            if (wallKickTest(hand, 0)) {
                draw = 1;
            }
        }
        if (getKeyDown('X') || getKeyDown(VK_UP)) {
            if (wallKickTest(hand, 1)) {
                draw = 1;
            }
        }
        if (getKeyDown('A')) {
            if (wallKickTest(hand, 2)) {
                draw = 1;
            }
        }
        if (getKeyDown('R')) {
            draw = 1;
            memset(field, 0, sizeof(field));
            bagrotate = 0;
            srand(time(NULL)+ct);
            bag[0] = generateRandomBag();
            bag[1] = generateRandomBag();
            hand = getBagRotated(bag, 0, bagrotate);
            hold = -1;
            isHolding = 0;
            resetPos();
            currot = 0;
            gravityReset();
        }
        if (getKeyDown(VK_SPACE)) {
            draw = 1;
            drop(ct);
        }
        if (getKeyDown('C') && !isHolding) {
            isHolding = 1;
            draw = 1;
            gravityReset();
            if (hold == -1) {
                hold = hand;
                bagrotate++;
                if (bagrotate > 7) {
                    bagrotate = 1;
                    bag[0] = bag[1];
                    bag[1] = generateRandomBag();
                }
                hand = getBagRotated(bag, 0, bagrotate);
            } else {
                int temp = hand;
                hand = hold;
                hold = temp;
            }
            resetPos();
            currot = 0;
        }

        if (draw) {
            draw = 0;

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_Rect rect;
            SetDrawColor(renderer, cgGrid);
            for (int i = 0;i < FIELD_SIZE_X;i++) {
                rect = (SDL_Rect) {102 + i*6, 12, 1, FIELD_SIZE_Y_NORMAL*6};
                renderRect(renderer, rect, 1);
            }
            for (int i = 0;i < FIELD_SIZE_Y_NORMAL;i++) {
                rect = (SDL_Rect) {97, 17 + i*6, FIELD_SIZE_X*6, 1};
                renderRect(renderer, rect, 1);
            }

            SetDrawColor(renderer, cWhite);
            rect = (SDL_Rect) {96, 11, 2+FIELD_SIZE_X*6, 2+FIELD_SIZE_Y_NORMAL*6};
            renderRect(renderer, rect, 1);

            rect = (SDL_Rect) {FIELD_SIZE_X*6+97, 19, 32, 20};
            renderRect(renderer, rect, 1);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+97, 19+19, 32, 20};
            renderRect(renderer, rect, 1);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+97, 19+19*2, 32, 20};
            renderRect(renderer, rect, 1);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+97, 19+19*3, 32, 20};
            renderRect(renderer, rect, 1);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+97, 19+19*4, 32, 20};
            renderRect(renderer, rect, 1);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+98, 11, 27, 8};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+125, 12, 1, 7};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+126, 13, 1, 6};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+127, 14, 1, 5};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {FIELD_SIZE_X*6+128, 15, 1, 4};
            renderFillRect(renderer, rect);
            renderFont(renderer, font7, "NEXT", FIELD_SIZE_X*6+101, 11, &cBlack, 0);
            rect = (SDL_Rect) {65, 19, 32, 20};
            renderRect(renderer, rect, 1);
            rect = (SDL_Rect) {69, 11, 27, 8};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {68, 12, 1, 7};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {67, 13, 1, 6};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {66, 14, 1, 5};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {65, 15, 1, 4};
            renderFillRect(renderer, rect);
            renderFont(renderer, font7, "HOLD", 73, 11, &cBlack, 0);

            if (hold != -1) {
                drawViewMino(renderer, hold+1, 72, 23, isHolding);
            }

            drawViewMino(renderer, getBagRotated(bag, 1, bagrotate)+1, FIELD_SIZE_X*6+104, 23, 0);
            drawViewMino(renderer, getBagRotated(bag, 2, bagrotate)+1, FIELD_SIZE_X*6+104, 23 + 19 * 1, 0);
            drawViewMino(renderer, getBagRotated(bag, 3, bagrotate)+1, FIELD_SIZE_X*6+104, 23 + 19 * 2, 0);
            drawViewMino(renderer, getBagRotated(bag, 4, bagrotate)+1, FIELD_SIZE_X*6+104, 23 + 19 * 3, 0);
            drawViewMino(renderer, getBagRotated(bag, 5, bagrotate)+1, FIELD_SIZE_X*6+104, 23 + 19 * 4, 0);

            drawField(renderer);

            int chacky = y;
            for (;;) {
                if (hand == 6) {
                    if ((FieldGetBits2(x, chacky-1) & 0b1111)) {
                        break;
                    }
                } else if (hand) {
                    if ((FieldGetBits3(x, chacky-1) & TETROMINOS[hand][currot])) {
                        break;
                    }
                } else {
                    if ((FieldGetBits4(x, chacky-1) & TETROMINOS[0][currot])) {
                        break;
                    }
                }
                chacky--;
            }
            int ghostdata = 0;
            FieldSetColor(&ghostdata, hand+1);
            FieldSetRot(&ghostdata, currot);
            setX(&ghostdata, x+1);
            setY(&ghostdata, chacky+1);
            drawMino(renderer, ghostdata, 1);

            int data = 0;
            FieldSetColor(&data, hand+1);
            FieldSetRot(&data, currot);
            setX(&data, x+1);
            setY(&data, y+1);
            drawMino(renderer, data, 0);

            /*
            renderFont(renderer, font7, "APM", FIELD_SIZE_X*6+100, 116, &cWhite, 0);
            char strAPM[7];
            snprintf(strAPM, sizeof(strAPM), "%.3f", 10.24f);
            renderFont(renderer, font7, strAPM, FIELD_SIZE_X*6+100, 125, &cWhite, 0);
            */

            if (lineClear) {
                renderFont(renderer, font7, getLineChar(lineClear), 95, 39, &cWhite, 1);
            }
            if (comboCount > 1) {
                char strCombo[17];
                snprintf(strCombo, sizeof(strCombo), "%d Combo", comboCount-1);
                renderFont(renderer, font7, strCombo, 95, 48, &cWhite, 1);
            }


            renderFont(renderer, font7, "0.0.0 DEMO", 0, 135, &cWhite, 0);
            SDL_RenderPresent(renderer);
        }
        SDL_Delay(1);
    }

ScnEnd:

    TTF_CloseFont(font7);
    TTF_CloseFont(font9);
    TTF_CloseFont(font11);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}