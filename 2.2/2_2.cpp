// drag_click_bolsonaro.cpp
// PNG-only + SDL_UserEvent + HUD com SDL_ttf (usando tiny.ttf)
// - Clique simples -> “irritado” (flash curto).
// - Arrasto -> “chorando” enquanto arrasta.
// - Idle (900 ms sem eventos) -> “dormindo”.
// - ESC durante gesto -> cancela e restaura a posição.
// - HUD: banner no topo indicando o evento atual (Clicou, Arrastando, Soltou, Cancelou, Idle).

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>

static const int WIN_W = 800;
static const int WIN_H = 600;
static const int BOX_SIZE = 160;

#define IDLE_TIMEOUT_MS 900
#define CLICK_FLASH_MS  180
#define DRAG_THRESHOLD  6

// Fonte do HUD
#define HUD_FONT_FILE "tiny.ttf"
#define HUD_FONT_SIZE 22   // ajuste se precisar (20–28 normalmente fica bom)

enum : int { EVT_CLICK = 1, EVT_DRAG_START = 2, EVT_DRAG_END = 3, EVT_CANCEL = 4, EVT_IDLE = 5 };

int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms) {
    const Uint32 antes = SDL_GetTicks();
    int isevt = SDL_WaitEventTimeout(evt, *ms);
    if (isevt) {
        Uint32 gasto = SDL_GetTicks() - antes;
        if (gasto > *ms) gasto = *ms;
        *ms -= gasto;
    }
    return isevt;
}

static inline bool pointInRect(int x, int y, const SDL_Rect& r) {
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

enum class UIState { IdleSleep, Armed, Dragging, ClickFlash };

// publica SDL_USEREVENT com code=EVT_*
static void push_user_event(int code) {
    SDL_Event ue; SDL_memset(&ue, 0, sizeof(ue));
    ue.type = SDL_USEREVENT;
    ue.user.type = SDL_USEREVENT;
    ue.user.code = code;
    SDL_PushEvent(&ue);
}

// ---------------- HUD com SDL_ttf ----------------
struct HudTTF {
    TTF_Font* font = nullptr;
    std::string lastLabel;
    SDL_Texture* textTex = nullptr;
    int textW = 0, textH = 0;
    SDL_Color bg = { 0,0,0,180 }; // banner translúcido

    SDL_Color colorForCode(int code) {
        switch (code) {
        case EVT_CLICK:      return SDL_Color{ 0,200, 70,255 }; // verde
        case EVT_DRAG_START: return SDL_Color{ 30,144,255,255 }; // azul
        case EVT_DRAG_END:   return SDL_Color{ 255,200,  0,255 }; // amarelo
        case EVT_CANCEL:     return SDL_Color{ 220, 60, 60,255 }; // vermelho
        case EVT_IDLE:       return SDL_Color{ 180,180,180,255 }; // cinza
        default:             return SDL_Color{ 255,255,255,255 };
        }
    }
    const char* textForCode(int code) {
        switch (code) {
        case EVT_CLICK:      return "Clicou! Ficou irritado??";
        case EVT_DRAG_START: return "Arrastando... Chora nao kkkkkk";
        case EVT_DRAG_END:   return "Soltou!";
        case EVT_CANCEL:     return "Cancelou! Nao tem como escapar...";
        case EVT_IDLE:       return "Idle (Vai ter bastante tempo para dormir)";
        default:             return "Pronto";
        }
    }

    void setLabel(SDL_Renderer* ren, const std::string& label, SDL_Color color) {
        if (!font) return;
        if (label == lastLabel && textTex) return;
        if (textTex) { SDL_DestroyTexture(textTex); textTex = nullptr; }
        lastLabel = label;

        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, label.c_str(), color);
        if (!surf) return;
        textTex = SDL_CreateTextureFromSurface(ren, surf);
        textW = surf->w; textH = surf->h;
        SDL_FreeSurface(surf);
    }

    // banner no topo com altura dinâmica baseada na fonte
    void draw(SDL_Renderer* ren) {
        int lineH = font ? TTF_FontHeight(font) : 20;
        int padY = 10;
        int h = (lineH + padY * 2);
        if (h < 36) h = 36;

        SDL_Rect banner{ 0, 0, WIN_W, h };
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, bg.a);
        SDL_RenderFillRect(ren, &banner);

        // “STATUS:”
        if (font) {
            SDL_Surface* s = TTF_RenderUTF8_Blended(font, "STATUS:", SDL_Color{ 200,200,200,255 });
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
                SDL_Rect dst{ 10, (banner.h - s->h) / 2, s->w, s->h };
                SDL_RenderCopy(ren, t, nullptr, &dst);
                SDL_DestroyTexture(t);
                SDL_FreeSurface(s);
            }
        }

        // texto do evento
        if (textTex) {
            int left = 110; // espaço para a palavra STATUS:
            SDL_Rect dst{ left, (banner.h - textH) / 2, textW, textH };
            if (dst.x + dst.w > WIN_W - 10) dst.w = WIN_W - 10 - dst.x;
            SDL_RenderCopy(ren, textTex, nullptr, &dst);
        }
    }

    void destroy() {
        if (textTex) { SDL_DestroyTexture(textTex); textTex = nullptr; }
        if (font) { TTF_CloseFont(font); font = nullptr; }
    }
};
// ---------------- /HUD ----------------

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 1; }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::fprintf(stderr, "IMG_Init (PNG): %s\n", IMG_GetError()); return 1;
    }
    if (TTF_Init() != 0) {
        std::fprintf(stderr, "TTF_Init: %s\n", TTF_GetError()); return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Drag, Click or Cancel",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!win || !ren) { std::fprintf(stderr, "CreateWindow/Renderer: %s\n", SDL_GetError()); return 1; }

    // HUD
    HudTTF hud;
    hud.font = TTF_OpenFont(HUD_FONT_FILE, HUD_FONT_SIZE);
    if (!hud.font) std::fprintf(stderr, "TTF_OpenFont('%s'): %s\n", HUD_FONT_FILE, TTF_GetError());
    hud.setLabel(ren, hud.textForCode(EVT_IDLE), hud.colorForCode(EVT_IDLE));

    // Texturas PNG
    SDL_Texture* texDormindo = IMG_LoadTexture(ren, "bolsonaro_dormindo.png");
    SDL_Texture* texIrritado = IMG_LoadTexture(ren, "bolsonaro_irritado.png");
    SDL_Texture* texChorando = IMG_LoadTexture(ren, "bolsonaro_chorando.png");
    if (!texDormindo || !texIrritado || !texChorando) {
        std::fprintf(stderr, "IMG_LoadTexture (PNG): %s\n", IMG_GetError()); return 1;
    }

    SDL_Rect box{ (WIN_W - BOX_SIZE) / 2, (WIN_H - BOX_SIZE) / 2, BOX_SIZE, BOX_SIZE };
    SDL_Rect boxOriginal = box;

    UIState state = UIState::IdleSleep;
    SDL_Texture* currentTex = texDormindo;

    bool gestureActive = false, isDragging = false;
    SDL_Point downPos{ 0,0 }, dragOffset{ 0,0 };

    Uint32 clickFlashRemainMs = 0;
    Uint32 clickFlashLastTick = 0;

    Uint32 idleCountdownMs = IDLE_TIMEOUT_MS;
    bool running = true;

    auto setHUDByEvent = [&](int code) {
        SDL_Color c = hud.colorForCode(code);
        hud.setLabel(ren, hud.textForCode(code), c);
        };

    while (running) {
        // Render
        SDL_SetRenderDrawColor(ren, 24, 24, 30, 255);
        SDL_RenderClear(ren);

        SDL_RenderCopy(ren, currentTex, nullptr, &box);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderDrawRect(ren, &box);

        // HUD no topo
        hud.draw(ren);

        SDL_RenderPresent(ren);

        // Eventos
        SDL_Event e;
        Uint32 waitMs = idleCountdownMs;
        if (AUX_WaitEventTimeoutCount(&e, &waitMs)) {
            idleCountdownMs = IDLE_TIMEOUT_MS;

            switch (e.type) {
            case SDL_QUIT:
                running = false; break;

            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE && gestureActive) {
                    box = boxOriginal; gestureActive = false; isDragging = false;
                    state = UIState::IdleSleep; currentTex = texDormindo;
                    clickFlashRemainMs = 0;
                    push_user_event(EVT_CANCEL);
                    setHUDByEvent(EVT_CANCEL);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int mx = e.button.x, my = e.button.y;
                    if (pointInRect(mx, my, box)) {
                        gestureActive = true; isDragging = false;
                        downPos = { mx, my }; dragOffset = { mx - box.x, my - box.y };
                        boxOriginal = box; state = UIState::Armed;
                    }
                }
                break;

            case SDL_MOUSEMOTION:
                if (gestureActive) {
                    int mx = e.motion.x, my = e.motion.y;
                    if (!isDragging) {
                        int dx = mx - downPos.x, dy = my - downPos.y;
                        if ((int)std::sqrt(double(dx * dx + dy * dy)) >= DRAG_THRESHOLD) {
                            isDragging = true; state = UIState::Dragging;
                            currentTex = texChorando;
                            clickFlashRemainMs = 0;
                            push_user_event(EVT_DRAG_START);
                            setHUDByEvent(EVT_DRAG_START);
                        }
                    }
                    if (isDragging) {
                        box.x = mx - dragOffset.x; box.y = my - dragOffset.y;
                        if (box.x < 0) box.x = 0; if (box.y < 0) box.y = 0;
                        if (box.x + box.w > WIN_W) box.x = WIN_W - box.w;
                        if (box.y + box.h > WIN_H) box.y = WIN_H - box.h;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT && gestureActive) {
                    if (!isDragging) {
                        state = UIState::ClickFlash;
                        currentTex = texIrritado;
                        clickFlashRemainMs = CLICK_FLASH_MS;
                        clickFlashLastTick = SDL_GetTicks();
                        box = boxOriginal;
                        push_user_event(EVT_CLICK);
                        setHUDByEvent(EVT_CLICK);
                    }
                    else {
                        state = UIState::IdleSleep;
                        currentTex = texChorando; 
                        push_user_event(EVT_DRAG_END);
                        setHUDByEvent(EVT_DRAG_END);
                    }
                    gestureActive = false; isDragging = false;
                }
                break;

            case SDL_USEREVENT:
                break;
            }
        }
        else {
            // Inatividade -> dormindo
            state = UIState::IdleSleep;
            currentTex = texDormindo;
            idleCountdownMs = IDLE_TIMEOUT_MS;
            push_user_event(EVT_IDLE);
            setHUDByEvent(EVT_IDLE);
        }

        // Timer do flash de clique
        if (clickFlashRemainMs > 0) {
            Uint32 now = SDL_GetTicks();
            Uint32 dt = now - clickFlashLastTick;
            clickFlashLastTick = now;
            if (dt > clickFlashRemainMs) dt = clickFlashRemainMs;
            clickFlashRemainMs -= dt;
            if (clickFlashRemainMs == 0) {
                state = UIState::IdleSleep;
                currentTex = texDormindo;
                setHUDByEvent(EVT_IDLE);
            }
        }
    }

    // cleanup
    hud.destroy();
    SDL_DestroyTexture(texDormindo);
    SDL_DestroyTexture(texIrritado);
    SDL_DestroyTexture(texChorando);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
