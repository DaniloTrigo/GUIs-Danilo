#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cassert>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

/* ========= Função auxiliar ========= */
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

/* ========= Constantes/Tuning ========= */
namespace cfg {
constexpr int   WIN_W = 1280;
constexpr int   WIN_H = 720;
constexpr int   FRAME = 128;         
constexpr int   GROUND_OFFSET = 48;  
constexpr int   SIDE_MARGIN   = 40;   
constexpr float SCALE = 2.0f;

constexpr float WALK_SPD = 120.f;
constexpr float RUN_SPD  = 220.f;
constexpr float ACCEL    = 900.f;
constexpr float DECEL    = 1000.f;

constexpr Uint32 MAX_WAIT_MS = 8;     
constexpr float  MAX_DT      = 0.033f;
}

/* ========= Texto  ========= */
struct TextLine { SDL_Texture* tex{}; int w{}, h{}; };

static TextLine make_text(SDL_Renderer* ren, TTF_Font* font, const char* str, SDL_Color color) {
    TextLine tl;
    SDL_Surface* sfc = TTF_RenderUTF8_Blended(font, str, color);
    if (!sfc) { std::printf("TTF_Render falhou: %s\n", TTF_GetError()); return tl; }
    tl.tex = SDL_CreateTextureFromSurface(ren, sfc);
    if (!tl.tex) {
        std::printf("CreateTextureFromSurface falhou\n");
        SDL_FreeSurface(sfc);
        return tl;
    }
    tl.w = sfc->w; tl.h = sfc->h;
    SDL_FreeSurface(sfc);
    return tl;
}

/* ========= Sprite / FSM ========= */
struct Anim {
    SDL_Texture* tex{};
    int tw{}, th{};
    int cols{};
    Uint32 frame_ms{100};
    bool loop{true};
    bool valid{false};
};

static bool load_anim(SDL_Renderer* ren, Anim* A, const char* path, Uint32 frame_ms, bool loop = true) {
    A->tex = IMG_LoadTexture(ren, path);
    if (!A->tex) {
        std::printf("Aviso: nao encontrei %s\n", path);
        A->valid = false;
        return false;
    }
    SDL_QueryTexture(A->tex, nullptr, nullptr, &A->tw, &A->th);
    A->cols = std::max(1, A->tw / cfg::FRAME);
    A->frame_ms = frame_ms;
    A->loop = loop;
    A->valid = true;
    std::printf("ok: %s size=%dx%d cols=%d\n", path, A->tw, A->th, A->cols);
    return true;
}

enum class State { IDLE, IDLE2, WALK, RUN, SHOT, ATTACK, JUMP, HURT, RECHARGE, DEAD, COUNT };
enum class Facing { RIGHT, LEFT };

/* ========= Painel de ajuda ========= */
static void draw_help(SDL_Renderer* rr, const std::vector<TextLine>& lines, SDL_Color bg) {
    if (lines.empty()) return;
    int pad = 10, lineh = 0, maxw = 0;
    for (const auto& tl : lines) { lineh = std::max(lineh, tl.h); maxw = std::max(maxw, tl.w); }
    const int totalh = static_cast<int>(lines.size()) * (lineh + 2) + pad * 2;
    const int totalw = maxw + pad * 2;

    SDL_Rect box{ 20, 20, totalw, totalh };
    SDL_SetRenderDrawBlendMode(rr, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rr, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(rr, &box);

    int x0 = box.x + pad, y0 = box.y + pad;
    for (const auto& tl : lines) {
        SDL_Rect d{ x0, y0, tl.w, tl.h };
        SDL_RenderCopy(rr, tl.tex, nullptr, &d);
        y0 += lineh + 2;
    }
}

int main(int, char**) {
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0);
    assert(IMG_Init(IMG_INIT_PNG) != 0);
    assert(TTF_Init() == 0);

    SDL_Window* win = SDL_CreateWindow(
        "City4 static BG + FSM (SDL2, sem SDL_Delay)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        cfg::WIN_W, cfg::WIN_H, SDL_WINDOW_SHOWN
    );
    assert(win);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(ren);

    // background
    SDL_Texture* bg = IMG_LoadTexture(ren, "City4.png");
    if (!bg) { std::printf("Nao foi possivel abrir City4.png: %s\n", IMG_GetError()); return 1; }
    SDL_Rect bgDst{ 0, 0, cfg::WIN_W, cfg::WIN_H };

    // animações
    std::vector<Anim> A(static_cast<size_t>(State::COUNT));
    auto S = [&](State s) -> Anim& { return A[static_cast<size_t>(s)]; };
    load_anim(ren, &S(State::IDLE),     "Idle.png",      120, true);
    load_anim(ren, &S(State::IDLE2),    "Idle_2.png",    120, true);
    load_anim(ren, &S(State::WALK),     "Walk.png",       90, true);
    load_anim(ren, &S(State::RUN),      "Run.png",        70, true);
    load_anim(ren, &S(State::SHOT),     "Shot.png",       70, false);
    load_anim(ren, &S(State::ATTACK),   "Attack_1.png",   80, false);
    load_anim(ren, &S(State::JUMP),     "Jump.png",       90, false);
    load_anim(ren, &S(State::HURT),     "Hurt.png",      110, false);
    load_anim(ren, &S(State::RECHARGE), "Recharge.png",  100, false);
    load_anim(ren, &S(State::DEAD),     "Dead.png",      140, false);

    auto has = [&](State s) { return S(s).valid; };

    // FSM/movimento
    State locomotion = has(State::IDLE) ? State::IDLE : (has(State::WALK) ? State::WALK : State::RUN);
    State st = locomotion;
    int cur = 0;
    Uint32 frameLeft = S(st).valid ? S(st).frame_ms : 80;
    Facing face = Facing::RIGHT;

    bool kL = false, kR = false, kShift = false;
    bool locked = false, dead = false;

    float v = 0.f, x = cfg::WIN_W * 0.2f;
    const int groundY = cfg::WIN_H - static_cast<int>(cfg::FRAME * cfg::SCALE) - cfg::GROUND_OFFSET;

    // TTF: overlay de ajuda
    const char* tryFonts[] = {
        "tiny.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        nullptr
    };
    TTF_Font* font = nullptr;
    for (int i = 0; tryFonts[i]; ++i) {
        font = TTF_OpenFont(tryFonts[i], 18);
        if (font) { std::printf("Fonte carregada: %s\n", tryFonts[i]); break; }
        else       std::printf("Falhou abrir: %s (%s)\n", tryFonts[i], TTF_GetError());
    }
    SDL_Color white{255,255,255,255}, black{0,0,0,180};
    bool showHelp = true;

    std::vector<TextLine> help;
    if (font) {
        const char* lines[] = {
            "Controles:",
            "Setas <-/-> : mover   |  Shift: correr   |  ESC: sair",
            "I: alterna Idle/Idle_2",
            "J: Attack_1   |  K: Shot   |  Espaco: Jump",
            "H: Hurt       |  R: Recharge",
            "L: Dead (congela no ultimo frame)",
            "F1: mostra/oculta este painel",
            nullptr
        };
        for (int i = 0; lines[i]; ++i) help.push_back(make_text(ren, font, lines[i], white));
    }

    
    Uint32 espera = cfg::MAX_WAIT_MS, last = SDL_GetTicks();
    SDL_Event ev; bool running = true;

    while (running) {
        if (AUX_WaitEventTimeoutCount(&ev, &espera)) {
            if (ev.type == SDL_QUIT) running = false;
            else if (ev.type == SDL_KEYDOWN) {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_ESCAPE) running = false;
                if (k == SDLK_F1) showHelp = !showHelp;

                if (!dead) {
                    if (k == SDLK_LEFT)  { kL = true;  face = Facing::LEFT;  }
                    if (k == SDLK_RIGHT) { kR = true;  face = Facing::RIGHT; }
                    if (k == SDLK_LSHIFT || k == SDLK_RSHIFT) kShift = true;

                    if (!locked) {
                        if (k == SDLK_j && has(State::ATTACK))   { st = State::ATTACK;   locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_k && has(State::SHOT))     { st = State::SHOT;     locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_SPACE && has(State::JUMP)) { st = State::JUMP;     locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_h && has(State::HURT))     { st = State::HURT;     locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_r && has(State::RECHARGE)) { st = State::RECHARGE; locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_l && has(State::DEAD))     { st = State::DEAD; dead = true; locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_i && has(State::IDLE2)) {
                            
                            State alt = State::IDLE2;
                            if (locomotion == alt && has(State::IDLE)) locomotion = State::IDLE;
                            else locomotion = alt;
                        }
                    }
                }
            } else if (ev.type == SDL_KEYUP) {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_LEFT)  kL = false;
                if (k == SDLK_RIGHT) kR = false;
                if (k == SDLK_LSHIFT || k == SDLK_RSHIFT) kShift = false;
            }
        } else {
            // ===== simulação =====
            const Uint32 nowTicks = SDL_GetTicks();
            float dt = (nowTicks - last) / 1000.f;
            last = nowTicks;
            if (dt > cfg::MAX_DT) dt = cfg::MAX_DT;

            // animação do estado atual
            if (espera >= frameLeft) {
                if (S(st).loop) cur = (cur + 1) % std::max(1, S(st).cols);
                else if (cur < S(st).cols - 1) cur++;
                frameLeft = S(st).frame_ms;

                if (!S(st).loop && cur == S(st).cols - 1) {
                    if (st != State::DEAD) { locked = false; st = locomotion; cur = 0; frameLeft = S(st).frame_ms; }
                }
            } else {
                frameLeft -= espera;
            }

            // movimento
            float target = 0.f;
            if (!locked && !dead) {
                const float maxspd = kShift ? cfg::RUN_SPD : cfg::WALK_SPD;
                if (kL ^ kR) target = (kL ? -maxspd : +maxspd);
            }
            if (target != 0.f) {
                const float dv = target - v;
                const float step = cfg::ACCEL * dt;
                if (std::fabs(dv) <= step) v = target;
                else v += (dv > 0 ? step : -step);
            } else {
                if (v > 0) { v = std::max(0.f, v - cfg::DECEL * dt); }
                if (v < 0) { v = std::min(0.f, v + cfg::DECEL * dt); }
            }

            // FSM locomocao
            if (!locked && !dead) {
                State ns = (std::fabs(v) < 1.0f) ? locomotion : (std::fabs(v) <= (cfg::WALK_SPD + 1.f) ? State::WALK : State::RUN);
                if (ns != st && has(ns)) { st = ns; cur = 0; frameLeft = S(st).frame_ms; }
            }

            // move o personagem dentro da janela
            x += v * dt;
            const int maxX = cfg::WIN_W - static_cast<int>(cfg::FRAME * cfg::SCALE) - cfg::SIDE_MARGIN;
            if (x < cfg::SIDE_MARGIN) x = static_cast<float>(cfg::SIDE_MARGIN);
            if (x > maxX) x = static_cast<float>(maxX);

            espera = (frameLeft < cfg::MAX_WAIT_MS ? frameLeft : cfg::MAX_WAIT_MS);
        }

        // ===== render =====
        SDL_RenderCopy(ren, bg, nullptr, &bgDst);

        const Anim* curAnim = &S(st);
        if (!curAnim->valid) {
            if      (has(State::RUN))  curAnim = &S(State::RUN);
            else if (has(State::WALK)) curAnim = &S(State::WALK);
        }
        SDL_Rect src{ cur * cfg::FRAME, 0, cfg::FRAME, cfg::FRAME };
        SDL_Rect dst{ static_cast<int>(x), groundY, static_cast<int>(cfg::FRAME * cfg::SCALE), static_cast<int>(cfg::FRAME * cfg::SCALE) };
        SDL_RenderCopyEx(ren, curAnim->tex, &src, &dst, 0.0, nullptr,
            (face == Facing::LEFT ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));

        // ajuda (F1) 
        if (font && showHelp) draw_help(ren, help, black);

        SDL_RenderPresent(ren);
    }

    // cleanup
    for (auto& an : A) if (an.tex) SDL_DestroyTexture(an.tex);
    if (bg) SDL_DestroyTexture(bg);
    for (auto& tl : help) if (tl.tex) SDL_DestroyTexture(tl.tex);
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
