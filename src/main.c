#include <stdlib.h>
#include <time.h>

#define bcase(cond, block) case cond: block break;

#define SDL_MAIN_USE_CALLBACKS 1
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "SDL3_ttf/SDL_ttf.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static TTF_Font *font = NULL;

static int wwidth = 640;
static int wheight = 480;

static int player_score = 0;

#define GAME_RECT_AMOUNT (4)
static SDL_FRect game_rects[GAME_RECT_AMOUNT]  = {0};
static SDL_FRect game_rect_dead = {0};
static float game_rect_lerp = 0;

static SDL_FRect game_rect_destruct = {0};

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Clamp(val, a, b) Max(Min(val, a), b)

bool rectPointInside(float x, float y, SDL_FRect *rect) {
    return 
        (x >= rect->x) &&
        (y >= rect->y) &&
        (x < rect->x + rect->w) &&
        (y < rect->y + rect->h);
}
void rectRandomize(float h, float w, SDL_FRect *rect) {
    rect->h = h;
    rect->w = w;
    rect->x = Max(rand() % (wwidth - 20), 20);
    rect->y = Max(rand() % (wheight - 20), 20);
}

void utilDrawCircle(SDL_Renderer* ren, int xc, int yc, int r) {
	int x = 0, y = r;
	int d = 3 - 2 * r;

	while (y >= x) {
		SDL_RenderPoint(ren, xc + x, yc + y);
		SDL_RenderPoint(ren, xc - x, yc + y);
		SDL_RenderPoint(ren, xc + x, yc - y);
		SDL_RenderPoint(ren, xc - x, yc - y);
		SDL_RenderPoint(ren, xc + y, yc + x);
		SDL_RenderPoint(ren, xc - y, yc + x);
		SDL_RenderPoint(ren, xc + y, yc - x);
		SDL_RenderPoint(ren, xc - y, yc - x);
		x++;
		if (d > 0) {
			y--;
			d += 4 * (x - y) + 10;
		}
		else {
			d += 4 * x + 6;
		}
	}
}
void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderPoint(renderer, cx + x, cy + y);
            }
        }
    }
}
void utilDrawFillCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

Uint32 gameRectRandomizeAllCallback(void *_, Uint32 __, Uint32 ___) {
    SDL_AddTimer(2000, gameRectRandomizeAllCallback, NULL);
    for(int i = 0; i < GAME_RECT_AMOUNT; ++i) {
        rectRandomize(20, 20, &game_rects[i]);
    }

    rectRandomize(20, 20, &game_rect_destruct);
    return 0;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Click On It!", NULL, NULL);

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Error: SDL: Couldn't init SDL3: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(!TTF_Init()) {
        SDL_Log("Error: TTF: Couldn't init TTF3: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(!SDL_CreateWindowAndRenderer(
        "Click On It!", 
        wwidth, wheight, 
        0, 
        &window, &renderer
    )) {
        SDL_Log("Error: SDL: Couldn't create window and/or renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    font = TTF_OpenFont("assets/fonts/RobotoMono-Regular.ttf", 16);
    if(!font) {
        SDL_Log("Error: TTF: Couldn't open font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    srand(time(NULL));

    gameRectRandomizeAllCallback(NULL, 0, 0);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch(event->type) {
        bcase(SDL_EVENT_QUIT, {
            return SDL_APP_SUCCESS;
        })
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    float mx, my;
    SDL_MouseButtonFlags mbtn = SDL_GetMouseState(&mx, &my);
    SDL_SetRenderDrawColor(renderer, 0x18, 0x18, 0x18, 0xFF);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xDD, 0xDD, 0x50, 0xFF);
    
    // for(int i = 0; i < 10; ++i) {
        //     rects[i].h = 20;
        //     rects[i].w = 20;
        //     rects[i].x = rand() % wwidth;
    //     rects[i].y = rand() % wheight;
    //     SDL_RenderFillRect(renderer, &rects[i]);
    // }
    
    for(int i = 0; i < GAME_RECT_AMOUNT; ++i) {
        SDL_RenderFillRect(renderer, &game_rects[i]);
    }
    
    if(mbtn & SDL_BUTTON_LMASK) {
        SDL_SetRenderDrawColor(renderer, 0xDD, 0xDD, 0xDD, 0xAA);
        utilDrawFillCircle(renderer, mx - 3, my - 3, 6);
        
        SDL_SetRenderDrawColor(renderer, 0xDD, 0xDD, 0x50, 0xFF);
        for(int i = 0; i < GAME_RECT_AMOUNT; ++i) {
            if(rectPointInside(mx, my, &game_rects[i])) {
                ++player_score;
                game_rect_dead = game_rects[i];
                game_rect_lerp = 300;
                rectRandomize(20, 20, &game_rects[i]);
                rectRandomize(20, 20, &game_rect_destruct);
            }

            SDL_RenderFillRect(renderer, &game_rects[i]);
        }
    }

    if(game_rect_lerp != 0) {
        float invert_lerp = 300 - game_rect_lerp;
        game_rect_dead.h = invert_lerp/3;
        game_rect_dead.w = invert_lerp/3;

        game_rect_dead.x = game_rect_dead.x - game_rect_dead.w/3;
        game_rect_dead.y = game_rect_dead.y - game_rect_dead.h/3;
        SDL_RenderFillRect(renderer, &game_rect_dead);

        game_rect_lerp--;
    }

    SDL_SetRenderDrawColor(renderer, 0xDD, 0x50, 0x50, 0xFF);
    SDL_RenderFillRect(renderer, &game_rect_destruct);

    if(mbtn & SDL_BUTTON_LMASK && rectPointInside(mx, my, &game_rect_destruct)) {
        char buf[128];
        SDL_snprintf(buf, SDL_arraysize(buf), "Game Over. Final Score: %04d", player_score);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game result", buf, window);
        return SDL_APP_SUCCESS;
    }

    {
        SDL_SetRenderDrawColor(renderer, 0xDD, 0xDD, 0xDD, 0xFF);
        char buf[13];
        SDL_snprintf(buf, SDL_arraysize(buf), "Score: %04d", player_score);
        SDL_Surface *text = TTF_RenderText_Blended(
            font, 
            buf, 0, 
            (SDL_Color){.r=0xFF, .g=0xFF, .b=0xFF,.a=0xFF}
        );
        if(!text) {
            SDL_Log("Error: TTF: Could not render text: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        texture = SDL_CreateTextureFromSurface(renderer, text);
        SDL_DestroySurface(text);

        SDL_FRect dst = {
            .x = 20,
            .y = 20
        };
        SDL_GetTextureSize(texture, &dst.w, &dst.h);
        SDL_RenderTexture(renderer, texture, NULL, &dst);

        SDL_DestroyTexture(texture);
    }

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {

}
