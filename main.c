#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define CANVAS_WIDTH 16*16 // 256
#define CANVAS_HEIGHT 9*16 // 144

const double PI = M_PI;

CRITICAL_SECTION inputcs;
int x = 0, y = 0;
int currot = 0;

_Bool focus = 0;

int running = 1;

typedef struct Camera {
    int x, y;
} Camera;
Camera cam = {0, 0};

_Bool isWindowFocused() {
    return GetForegroundWindow() == GetActiveWindow();
}

static _Bool previousKeyState[256] = { 0 };
static _Bool getKey(int key) {
    if (!isWindowFocused()) {
        return 0;
    }
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}
static _Bool getKeyDown(int key) {
    if (!isWindowFocused()) {
        return 0;
    }
    _Bool isDown = (GetAsyncKeyState(key) & 0x8000) != 0;
    _Bool wasDown = previousKeyState[key];

    previousKeyState[key] = isDown;

    return (isDown && !wasDown);
}
static _Bool getKeyUp(int key) {
    if (!isWindowFocused()) {
        return 0;
    }
    _Bool isDown = (GetAsyncKeyState(key) & 0x8000) != 0;
    _Bool wasDown = previousKeyState[key];

    previousKeyState[key] = isDown;

    return (!isDown && wasDown);
}

DWORD WINAPI handleKeyboardInput(LPVOID lpParam) {
    while (running) {
        EnterCriticalSection(&inputcs);
        if (focus) {
            
        }
        LeaveCriticalSection(&inputcs);
    }
    return 0;
}

static int renderFont(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color* color) {
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

    SDL_Rect rect = { x - cam.x, y - cam.y, surface->w, surface->h };

    if (SDL_RenderCopy(renderer, texture, NULL, &rect) != 0) {
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        return 1;
    }

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return 0;
}

static int renderRect(SDL_Renderer* renderer, SDL_Rect rect, int thickness) {
    int x = rect.x - cam.x;
    int y = rect.y - cam.y;

    if (thickness > 0) {
        for (int i = 0; i < thickness; ++i) {
            SDL_Rect border = { x + i, y + i, rect.w - 2 * i, rect.h - 2 * i };
            if (SDL_RenderDrawRect(renderer, &border) != 0) {
                return 1;
            }
        }
    }
    else {
        SDL_Rect newrect = { x, y, rect.w, rect.h };
        if (SDL_RenderDrawRect(renderer, &newrect) != 0) {
            return 1;
        }
    }

    return 0;
}

static int renderFillRect(SDL_Renderer* renderer, SDL_Rect rect) {
    int x = rect.x - cam.x;
    int y = rect.y - cam.y;

    SDL_Rect newrect = {x, y, rect.w, rect.h};
    if (SDL_RenderFillRect(renderer, &newrect) != 0) {
        return 1;
    }

    return 0;
}

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

/*
* //////////////////
* //  C  O  R  E  //
* //////////////////
*/

const int TETROMINOS[6][4] = {
    {//I, 4x4
        0b0000000011110000,
        0b0100010001000100,
        0b0000111100000000,
        0b0010001000100010
    }, {//J
        0b000111001,
        0b010010110,
        0b100111000,
        0b011010010,
    }, {//L
        0b000111100,
        0b110010010,
        0b001111000,
        0b010010011
    }, {//S
        0b000011110,
        0b100110010,
        0b011110000,
        0b010011001
    }, {//T
        0b000111010,
        0b010110010,
        0b010111000,
        0b010011010
    }, {//Z
        0b000110011,
        0b010110100,
        0b110011000,
        0b001011010
    }
};

#define COLOR_MASK    0x1F
#define DIR_MASK      0x3
#define SHAPE_MASK    0x7
#define TYPE_MASK     0x3
#define X_MASK        0x3FF
#define Y_MASK        0x3FF

#define COLOR_SHIFT   0
#define DIR_SHIFT     5
#define SHAPE_SHIFT   7
#define TYPE_SHIFT    10
#define X_SHIFT       12
#define Y_SHIFT       22

static inline unsigned int getColor(unsigned int data) {
    return data & COLOR_MASK;
}
static inline unsigned int getRot(unsigned int data) {
    return (data >> DIR_SHIFT) & DIR_MASK;
}
static inline unsigned int getShape(unsigned int data) {
    return (data >> SHAPE_SHIFT) & SHAPE_MASK;
}
static inline unsigned int getType(unsigned int data) {
    return (data >> TYPE_SHIFT) & TYPE_MASK;
}
static inline unsigned int getX(unsigned int data) {
    return (data >> X_SHIFT) & X_MASK;
}
static inline unsigned int getY(unsigned int data) {
    return (data >> Y_SHIFT) & Y_MASK;
}

static inline void setColor(unsigned int* data, unsigned int color) {
    *data = (*data & ~COLOR_MASK) | (color & COLOR_MASK);
}
static inline void setRot(unsigned int* data, unsigned int dir) {
    *data = (*data & ~(DIR_MASK << DIR_SHIFT)) | ((dir & DIR_MASK) << DIR_SHIFT);
}
static inline void setShape(unsigned int* data, unsigned int shape) {
    *data = (*data & ~(SHAPE_MASK << SHAPE_SHIFT)) | ((shape & SHAPE_MASK) << SHAPE_SHIFT);
}
static inline void setType(unsigned int* data, unsigned int type) {
    *data = (*data & ~(TYPE_MASK << TYPE_SHIFT)) | ((type & TYPE_MASK) << TYPE_SHIFT);
}
static inline void setX(unsigned int* data, unsigned int x) {
    *data = (*data & ~(X_MASK << X_SHIFT)) | ((x & X_MASK) << X_SHIFT);
}
static inline void setY(unsigned int* data, unsigned int y) {
    *data = (*data & ~(Y_MASK << Y_SHIFT)) | ((y & Y_MASK) << Y_SHIFT);
}

#define FIELD_SIZE_X 10
#define FIELD_SIZE_Y 32

unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X] = {0, };


//Kick - 3bit ... 000 = -3, 001 = -2, ...101 = 2

//0~31, 1>0 2>1 3>2 0>3 | 32~63, 3>0 0>1 1>2 2>3
static const unsigned int wallkickI[7] = {
    0b011100001010100101011010011101,
    0b011100011001100001001100011001,
    0b101010011101011010101100010001,
    0b100001001100011001011100010101, 
    0b011010101100010001011100011001,
    0b011010011101010101101010011101,
    0b001010100101
};
//0~31, 1>0 2>1 3>2 0>3 | 32~63, 3>0 0>1 1>2 2>3
static const unsigned int wallkickJLSTZ[7] = {
    0b011010101100101011010100011100, 
    0b010010011010001010001011100010,
    0b001011100100011100101010101011,
    0b101010101011010010011010001100,
    0b011100001010001011100010011010,
    0b100100011100101100101011010100,
    0b001100001011
};
//40, 2>0 3>>1 0>>2 1>>3
static const unsigned int wallkickI180[4] = {
    0b010011011001011010011101011100,
    0b011100001011010011101011100011,
    0b100011011101011100011001011010,
    0b011010001011010011101011100011
};
//88, 2>0 3>>1 0>>2 1>>3
static const unsigned int wallkickJLSTZ180[9] = {
    0b011100010001010010011001011010,
    0b011000100011010101010100011101,
    0b101100100100101011100011011110,
    0b011010001100010100001011010011,
    0b100100011101011100000011110011,
    0b100001100010011001011010100101,
    0b101011100011011000011110010011,
    0b010010001011010011101010100010,
    0b000011110011011100001010
};

static int extractWallkick(const int* table, int kickIndex) {
    return (((table[kickIndex / 10] >> ((kickIndex * 3) % 30)) & 0x7) - 3);
}
static int getFieldBits(int x, int y) {
    return (y < 0 || y >= FIELD_SIZE_Y || x < 0 || x >= FIELD_SIZE_X || field[y][x]) ? 1 : 0;
}

static int getFieldBits2(int x, int y) {
    return 
        (getFieldBits(x, y)         << 0) |
        (getFieldBits(x + 1, y)     << 1) |
        (getFieldBits(x, y + 1)     << 2) |
        (getFieldBits(x + 1, y + 1) << 3);
}

static int getFieldBits3(int x, int y) {
    return 
        (getFieldBits(x - 1, y + 1) << 0) |
        (getFieldBits(x, y + 1)     << 1) |
        (getFieldBits(x + 1, y + 1) << 2) |
        (getFieldBits(x - 1, y)     << 3) |
        (getFieldBits(x, y)         << 4) |
        (getFieldBits(x + 1, y)     << 5) |
        (getFieldBits(x - 1, y - 1) << 6) |
        (getFieldBits(x, y - 1)     << 7) |
        (getFieldBits(x + 1, y - 1) << 8);
}

static int getFieldBits4(int x, int y) {
    return 
        (getFieldBits(x - 1, y + 1) << 0)  |
        (getFieldBits(x, y + 1)     << 1)  |
        (getFieldBits(x + 1, y + 1) << 2)  |
        (getFieldBits(x + 2, y + 1) << 3)  |
        (getFieldBits(x - 1, y)     << 4)  |
        (getFieldBits(x, y)         << 5)  |
        (getFieldBits(x + 1, y)     << 6)  |
        (getFieldBits(x + 2, y)     << 7)  |
        (getFieldBits(x - 1, y - 1) << 8)  |
        (getFieldBits(x, y - 1)     << 9)  |
        (getFieldBits(x + 1, y - 1) << 10) |
        (getFieldBits(x + 2, y - 1) << 11) |
        (getFieldBits(x - 1, y - 2) << 12) |
        (getFieldBits(x, y - 2)     << 13) |
        (getFieldBits(x + 1, y - 2) << 14) |
        (getFieldBits(x + 2, y - 2) << 15);
}


static _Bool wallKickTest(int mino, int rot) {
    if (mino > 5) {
        return 0;
    }
    const unsigned int* table;
    int nextrot = (currot + (rot == 0 ? 3 : rot)) % 4;
    
    if (mino) {
        if (rot == 2) {
            if (getFieldBits3(x, y) & TETROMINOS[mino][nextrot]) {
                for (int i = 0; i < 11; i++) {
                    int ty = extractWallkick(wallkickJLSTZ180, nextrot * 22 + i * 2) + x;
                    int tx = extractWallkick(wallkickJLSTZ180, nextrot * 22 + i * 2 + 1) + y;
                    if (!(getFieldBits3(ty, tx) & TETROMINOS[mino][nextrot])) {
                        currot = nextrot;
                        x = ty;
                        y = tx;
                        return 1;
                    }
                }
            } else {
                currot = nextrot;
            }
        } else {
            if (getFieldBits3(x, y) & TETROMINOS[mino][nextrot]) {
                for (int i = 0; i < 4; i++) {
                    int ty = extractWallkick(wallkickJLSTZ, nextrot * 8 + i * 2 + (rot * 32)) + x;
                    int tx = extractWallkick(wallkickJLSTZ, nextrot * 8 + i * 2 + 1 + (rot * 32)) + y;
                    if (!(getFieldBits3(ty, tx) & TETROMINOS[mino][nextrot])) {
                        currot = nextrot;
                        x = ty;
                        y = tx;
                        return 1;
                    }
                }
            } else {
                currot = nextrot;
                return 1;
            }
        }
    } else {
        if (rot == 2) {
            if (getFieldBits4(x, y) & TETROMINOS[mino][nextrot]) {
                for (int i = 0; i < 5; i++) {
                    int ty = extractWallkick(wallkickI180, nextrot * 10 + i * 2) + x;
                    int tx = extractWallkick(wallkickI180, nextrot * 10 + i * 2 + 1) + y;
                    if (!(getFieldBits4(ty, tx) & TETROMINOS[mino][nextrot])) {
                        currot = nextrot;
                        x = ty;
                        y = tx;
                        return 1;
                    }
                }
            } else {
                currot = nextrot;
                return 1;
            }
        } else {
            if (getFieldBits4(x, y) & TETROMINOS[mino][nextrot]) {
                for (int i = 0; i < 4; i++) {
                    int ty = extractWallkick(wallkickI, nextrot * 8 + i * 2 + (rot * 32)) + x;
                    int tx = extractWallkick(wallkickI, nextrot * 8 + i * 2 + 1 + (rot * 32)) + y;
                    if (!(getFieldBits4(ty, tx) & TETROMINOS[mino][nextrot])) {
                        if (rot) {
                            currot++;
                        } else {
                            currot--;
                        }
                        x = ty;
                        y = tx;
                        return 1;
                    }
                }
            } else {
                currot = nextrot;
                return 1;
            }
        }
    }
    return 0;
}

#define BAG_SIZE 7
inline unsigned int getBag(unsigned int bag, int index) {
    return (bag >> (index * 3)) & 0x7;
}
unsigned int getBagRotated(const unsigned int* bag, int index, int rotate) {
    return getBag(bag[(index + rotate) / BAG_SIZE], (index + rotate) % BAG_SIZE);
}
int bag[2] = {0, };
int generateRandomBag() {
    int bagData = 0;
    int blocks[BAG_SIZE] = {0, 1, 2, 3, 4, 5, 6};

    for (int i = BAG_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = blocks[i];
        blocks[i] = blocks[j];
        blocks[j] = temp;
    }

    for (int i = 0; i < BAG_SIZE; i++) {
        bagData |= (blocks[i] << (i * 3));
    }

    return bagData;
}

_Bool isHolding = 0;
int hand;
int hold = -1;
int forcedroptimer = 0;
int bagrotate = 0;

//Handling ARR DAS SDF
int handling[3] = {30, 200, 30};
int handlingclock[5] = {0, 0, 0, 0, 0};
_Bool dasactive[2] = {0, 0};

int Forcedrop = 900;
int ForcedropTimer = 0;

static _Bool isCanMoveAt(int deg) {
    int xp = x;
    int yp = y;
    switch (deg) {
        case 0:
            yp--;
            break;
        case 1:
            xp--;
            break;
        case 2:
            xp++;
            break;
    }
    if (hand == 6) {
        if (!(getFieldBits2(xp, yp) & 0b1111)) {
            return 1;
        }
    } else if (hand) {
        if (!(getFieldBits3(xp, yp) & TETROMINOS[hand][currot])) {
            return 1;
        }
    } else {
        if (!(getFieldBits4(xp, yp) & TETROMINOS[0][currot])) {
            return 1;
        }
    }
    return 0;
}

static void drop() {
    int downy = y;
    for (;;) {
        if (hand == 6) {
            if ((getFieldBits2(x, downy-1) & 0b1111)) {
                break;
            }
        } else if (hand) {
            if ((getFieldBits3(x, downy-1) & TETROMINOS[hand][currot])) {
                break;
            }
        } else {
            if ((getFieldBits4(x, downy-1) & TETROMINOS[0][currot])) {
                break;
            }
        }
        downy--;
    }

    int data = 0;
    setColor(&data, hand+1);

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
        }
    }

    bagrotate++;
    if (bagrotate > 7) {
        bagrotate = 1;
        bag[0] = bag[1];
        bag[1] = generateRandomBag();
    }
    isHolding = 0;
    hand = getBagRotated(bag, 0, bagrotate);
    x = 4;
    y = 20;
    currot = 0;
}

static void gravityReset() {
    ForcedropTimer = clock() + Forcedrop;
}

static _Bool gravity(int clock) {
    if (ForcedropTimer < clock) {
        if (isCanMoveAt(0)) {
            gravityReset();
            y--;
        } else {
            drop();
        }
        return 1;
    }
    return 0;
}

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

static int drawField(SDL_Renderer* renderer) {
    for (int i = 0; i < FIELD_SIZE_Y; i++) {
        for (int j = 0; j < FIELD_SIZE_X; j++) {
            if (!field[i][j]) {
                continue;
            }
            SetDrawColor(renderer, ColorByMino(getColor(field[i][j])));
            SDL_Rect rect = {97 + 6 * j, 126 - 6 * i, 6, 6};
            renderFillRect(renderer, rect);
        }
    }
    return 0;
}

static int drawMino(SDL_Renderer* renderer, int data, _Bool ghost) {
    if (ghost) {
        SDL_Color color = ColorByMino(getColor(data));
        color.r -= 60;
        color.g -= 60;
        color.b -= 60;
        SetDrawColor(renderer, color);
    } else {
        SetDrawColor(renderer, ColorByMino(getColor(data)));
    }
    int bitIndex = 0;
    if (getColor(data) == 1) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (TETROMINOS[0][getRot(data)] & (1 << bitIndex)) {
                    SDL_Rect rect = {97 + (getX(data) - 1) * 6 + j * 6 - 6, 126 - (getY(data) - 1) * 6 + i * 6 - 6, 6, 6};
                    renderFillRect(renderer, rect);
                }
                bitIndex++;
            }
        }
    } else if (getColor(data) == 7) {
        SDL_Rect rect = {97 + (getX(data) - 1) * 6, 126 - (getY(data) - 1) * 6 - 6, 12, 12};
        renderFillRect(renderer, rect);
    } else {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (TETROMINOS[getColor(data)-1][getRot(data)] & (1 << bitIndex)) {
                    SDL_Rect rect = {97 + (getX(data) - 1) * 6 + j * 6 - 6, 126 - (getY(data) - 1) * 6 + i * 6 - 6, 6, 6};
                    renderFillRect(renderer, rect);
                }
                bitIndex++;
            }
        }
    }
    return 0;
}

static int drawViewMino(SDL_Renderer* renderer, int mino, int x, int y, _Bool forcegray) {
    if (forcegray) {
        SetDrawColor(renderer, ColorByMino(8));
    } else {
        SetDrawColor(renderer, ColorByMino(mino));
    }
    
    SDL_Rect rect;
    switch (mino) {
        case 1: 
            rect = (SDL_Rect) {x-3, y+3, 24, 6};
            renderFillRect(renderer, rect);
            break;
        case 2:
            rect = (SDL_Rect) {x, y, 6, 6};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {x, y+6, 18, 6};
            renderFillRect(renderer, rect);
            break;
        case 3:
            rect = (SDL_Rect) {x+12, y, 6, 6};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {x, y+6, 18, 6};
            renderFillRect(renderer, rect);
            break;
        case 4:
            rect = (SDL_Rect) {x+6, y, 12, 6};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {x, y+6, 12, 6};
            renderFillRect(renderer, rect);
            break;
        case 5:
            rect = (SDL_Rect) {x+6, y, 6, 6};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {x, y+6, 18, 6};
            renderFillRect(renderer, rect);
            break;
        case 6:
            rect = (SDL_Rect) {x, y, 12, 6};
            renderFillRect(renderer, rect);
            rect = (SDL_Rect) {x+6, y+6, 12, 6};
            renderFillRect(renderer, rect);
            break;
        case 7:
            rect = (SDL_Rect) {x+3, y, 12, 12};
            renderFillRect(renderer, rect);
            break;
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

    //InitializeCriticalSection(&inputcs);
    //HANDLE inputthread = CreateThread(0, 0, handleKeyboardInput, 0, 0, 0);
    //if (!inputthread) {
    //    return 1;
    //}

    //º¯¼ö

    _Bool nab = 0;
    int menusel = 0;
    _Bool issetting = 0;
    int scn = 0;
    register _Bool draw = 1;

    x = 4;
    y = 20;

    //³¡

    srand(time(NULL)+clock());
    bag[0] = generateRandomBag();
    bag[1] = generateRandomBag();
    hand = getBagRotated(bag, 0, bagrotate);

    SDL_Event e;

        clock_t ct = clock();

        //EnterCriticalSection(&inputcs);
        //focus = isWindowFocused();
        //LeaveCriticalSection(&inputcs);

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
        Scn0:
        for (;;) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = 0;
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
                            running = 0;
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
                        renderFont(renderer, font11, "- Run -", 106, 50, &cGreen);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        renderFont(renderer, font11, "Run", 117, 50, &cWhite);
                    }
                    if (menusel == 2) {
                        renderFont(renderer, font11, "- Options -", 93, 66, &cGreen);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        renderFont(renderer, font11, "Options", 104, 66, &cWhite);
                    }
                    if (menusel == 3) {
                        renderFont(renderer, font11, "- Exit -", 105, 82, &cGreen);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        renderFont(renderer, font11, "Exit", 116, 82, &cWhite);
                    }

                    if (nab) {
                        renderFont(renderer, font7, "Not available", 0, -1, &cRed);
                    }

                    renderFont(renderer, font7, "0.0.0 DEMO", 0, 135, &cWhite);
                    SDL_RenderPresent(renderer);
                }
                SDL_Delay(1);
            }
        }
        Scn1:
        for (;;) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = 0;
                    goto ScnEnd;
                }
            }
            ct = clock();
            if (gravity(ct)) {
                draw = 1;
            }
            if (getKey(VK_DOWN)) {
                if (isCanMoveAt(0) && handlingclock[2] < ct) {
                    gravityReset();
                    handlingclock[2] = handling[2] + ct;
                    if (handling[2] <= 0) {
                        while (isCanMoveAt(0)) {
                            draw = 1;
                            y--;
                        }
                    } else {
                        draw = 1;
                        y--;
                    }
                }
            }
            if (getKey(VK_LEFT)) {
                if (isCanMoveAt(1)) {
                    if (handlingclock[0] < ct && dasactive[0] && handlingclock[3] < ct) {
                        handlingclock[0] = ct + handling[0];
                        if (handling[0] <= 0) {
                            while (isCanMoveAt(1)) {
                                draw = 1;
                                x -= 1;
                            }
                        } else {
                            draw = 1;
                            x -= 1;
                        }
                    } else if (!dasactive[0]) {
                        draw = 1;
                        dasactive[0] = 1;
                        x -= 1;
                    }
                }
            } else {
                dasactive[0] = 0;
                handlingclock[3] = ct + handling[1];
            }
            if (getKey(VK_RIGHT)) {
                if (isCanMoveAt(2)) {
                    if (handlingclock[1] < ct && dasactive[1] && handlingclock[4] < ct) {
                        handlingclock[1] = ct + handling[0];
                        if (handling[1] <= 0) {
                            while (isCanMoveAt(1)) {
                                draw = 1;
                                x += 1;
                            }
                        } else {
                            draw = 1;
                            x += 1;
                        }
                    } else if (!dasactive[1]) {
                        draw = 1;
                        dasactive[1] = 1;
                        x += 1;
                    }
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
                x = 4;
                y = 20;
                currot = 0;
                gravityReset();
            }
            if (getKeyDown(VK_SPACE)) {
                draw = 1;
                drop();
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
                x = 4;
                y = 20;
                currot = 0;
            }

            if (draw) {
                draw = 0;

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                SDL_Rect rect;
                SetDrawColor(renderer, cgGrid);
                for (int i = 0;i < 10;i++) {
                    rect = (SDL_Rect) {102 + i*6, 12, 1, 120};
                    renderRect(renderer, rect, 1);
                }
                for (int i = 0;i < 20;i++) {
                    rect = (SDL_Rect) {97, 17 + i*6, 60, 1};
                    renderRect(renderer, rect, 1);
                }

                SetDrawColor(renderer, cWhite);
                rect = (SDL_Rect) {96, 11, 2+10*6, 2+20*6};
                renderRect(renderer, rect, 1);

                rect = (SDL_Rect) {157, 19, 32, 20};
                renderRect(renderer, rect, 1);
                rect = (SDL_Rect) {157, 19+19, 32, 20};
                renderRect(renderer, rect, 1);
                rect = (SDL_Rect) {157, 19+19*2, 32, 20};
                renderRect(renderer, rect, 1);
                rect = (SDL_Rect) {157, 19+19*3, 32, 20};
                renderRect(renderer, rect, 1);
                rect = (SDL_Rect) {157, 19+19*4, 32, 20};
                renderRect(renderer, rect, 1);
                rect = (SDL_Rect) {158, 11, 27, 8};
                renderFillRect(renderer, rect);
                rect = (SDL_Rect) {185, 12, 1, 7};
                renderFillRect(renderer, rect);
                rect = (SDL_Rect) {186, 13, 1, 6};
                renderFillRect(renderer, rect);
                rect = (SDL_Rect) {187, 14, 1, 5};
                renderFillRect(renderer, rect);
                rect = (SDL_Rect) {188, 15, 1, 4};
                renderFillRect(renderer, rect);
                renderFont(renderer, font7, "NEXT", 162, 11, &cBlack);
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
                renderFont(renderer, font7, "HOLD", 73, 11, &cBlack);

                drawField(renderer);

                if (hold != -1) {
                    drawViewMino(renderer, hold+1, 72, 23, isHolding);
                }

                drawViewMino(renderer, getBagRotated(bag, 1, bagrotate)+1, 164, 23, 0);
                drawViewMino(renderer, getBagRotated(bag, 2, bagrotate)+1, 164, 23 + 19 * 1, 0);
                drawViewMino(renderer, getBagRotated(bag, 3, bagrotate)+1, 164, 23 + 19 * 2, 0);
                drawViewMino(renderer, getBagRotated(bag, 4, bagrotate)+1, 164, 23 + 19 * 3, 0);
                drawViewMino(renderer, getBagRotated(bag, 5, bagrotate)+1, 164, 23 + 19 * 4, 0);

                int chacky = y;
                for (;;) {
                    if (hand == 6) {
                        if ((getFieldBits2(x, chacky-1) & 0b1111)) {
                            break;
                        }
                    } else if (hand) {
                        if ((getFieldBits3(x, chacky-1) & TETROMINOS[hand][currot])) {
                            break;
                        }
                    } else {
                        if ((getFieldBits4(x, chacky-1) & TETROMINOS[0][currot])) {
                            break;
                        }
                    }
                    chacky--;
                }
                int ghostdata = 0;
                setColor(&ghostdata, hand+1);
                setRot(&ghostdata, currot);
                setX(&ghostdata, x+1);
                setY(&ghostdata, chacky+1);
                drawMino(renderer, ghostdata, 1);

                int data = 0;
                setColor(&data, hand+1);
                setRot(&data, currot);
                setX(&data, x+1);
                setY(&data, y+1);
                drawMino(renderer, data, 0);

                renderFont(renderer, font7, "0.0.0 DEMO", 0, 135, &cWhite);
                SDL_RenderPresent(renderer);
            }
            SDL_Delay(1);
        }
        
    ScnEnd:

    //WaitForSingleObject(inputthread, INFINITE);
    //CloseHandle(inputthread);

    TTF_CloseFont(font7);
    TTF_CloseFont(font9);
    TTF_CloseFont(font11);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}