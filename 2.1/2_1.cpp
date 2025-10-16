// city4_multiclick_bigshot.cpp
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cassert>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

/* ========= Função auxiliar (exatamente como você pediu) ========= */
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
    constexpr int   SIDE_MARGIN = 40;
    constexpr float SCALE = 2.0f;

    constexpr float WALK_SPD = 120.f;
    constexpr float RUN_SPD = 220.f;
    constexpr float ACCEL = 900.f;
    constexpr float DECEL = 1000.f;

    constexpr Uint32 MAX_WAIT_MS = 8;
    constexpr float  MAX_DT = 0.033f;

    constexpr Uint32 MULTI_SILENCE_MS = 250;
    constexpr int    MULTI_TOL = 3;

    /* === Offsets do cano (ajuste fino) ===
       Frações do retângulo do personagem + correção em pixels pós-escala. */
    constexpr float MUZZLE_Y_F = 0.60f;  // 0=topo, 1=base; aumente p/ descer
    constexpr float MUZZLE_X_R_F = 0.83f;  // olhando p/ direita
    constexpr float MUZZLE_X_L_F = 0.17f;  // olhando p/ esquerda
    constexpr int   MUZZLE_Y_PX = +10;    // ajuste extra Y (px)
    constexpr int   MUZZLE_X_PX_R = +6;     // ajuste extra X quando RIGHT
    constexpr int   MUZZLE_X_PX_L = -6;     // ajuste extra X quando LEFT

    /* ---- BigShot (projétil do multiclique) ---- */
    constexpr float BIGSHOT_BASE_SPEED = 900.f;
    constexpr float BIGSHOT_BASE_RANGE = 700.f;
    constexpr float BIGSHOT_ADD_PER_N = 250.f;
    constexpr float BIGSHOT_SCALE_MIN = 0.22f;
    constexpr float BIGSHOT_SCALE_STEP = 0.06f;
}

/* ========= Texto (overlay de ajuda) ========= */
struct TextLine { SDL_Texture* tex{}; int w{}, h{}; };
static TextLine make_text(SDL_Renderer* ren, TTF_Font* font, const char* str, SDL_Color color) {
    TextLine tl;
    SDL_Surface* sfc = TTF_RenderUTF8_Blended(font, str, color);
    if (!sfc) { std::printf("TTF_Render falhou: %s\n", TTF_GetError()); return tl; }
    tl.tex = SDL_CreateTextureFromSurface(ren, sfc);
    if (!tl.tex) { std::printf("CreateTextureFromSurface falhou\n"); SDL_FreeSurface(sfc); return tl; }
    tl.w = sfc->w; tl.h = sfc->h; SDL_FreeSurface(sfc); return tl;
}
static void draw_help(SDL_Renderer* rr, const std::vector<TextLine>& lines, SDL_Color bg) {
    if (lines.empty()) return;
    int pad = 10, lineh = 0, maxw = 0;
    for (const auto& tl : lines) { lineh = std::max(lineh, tl.h); maxw = std::max(maxw, tl.w); }
    SDL_Rect box{ 20,20, maxw + pad * 2, int(lines.size()) * (lineh + 2) + pad * 2 };
    SDL_SetRenderDrawBlendMode(rr, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rr, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(rr, &box);
    int x = box.x + pad, y = box.y + pad;
    for (const auto& tl : lines) { SDL_Rect d{ x,y,tl.w,tl.h }; SDL_RenderCopy(rr, tl.tex, nullptr, &d); y += lineh + 2; }
}

/* ========= Sprite / FSM do personagem ========= */
struct Anim {
    SDL_Texture* tex{}; int tw{}, th{}, cols{};
    Uint32 frame_ms{ 100 }; bool loop{ true }; bool valid{ false };
};
static bool load_anim(SDL_Renderer* ren, Anim* A, const char* path, Uint32 frame_ms, bool loop = true) {
    A->tex = IMG_LoadTexture(ren, path);
    if (!A->tex) { std::printf("Aviso: nao encontrei %s\n", path); return A->valid = false; }
    SDL_QueryTexture(A->tex, nullptr, nullptr, &A->tw, &A->th);
    A->cols = std::max(1, A->tw / cfg::FRAME);
    A->frame_ms = frame_ms; A->loop = loop; return A->valid = true;
}
enum class State { IDLE, IDLE2, WALK, RUN, SHOT, ATTACK, JUMP, HURT, RECHARGE, DEAD, COUNT };
enum class Facing { RIGHT, LEFT };

/* ========= Explosão (10 PNGs isolados) ========= */
static SDL_Texture* gExpl[10]{};

/* ========= Projétil BigShot ========= */
struct BigShot {
    float x{}, y{}, vx{};
    float scale{ 0.35f };
    float traveled{ 0.f }, maxRange{ 800.f };
    int frame{ 0 }; Uint32 frame_ms{ 40 };
    bool alive{ false };
};
static std::vector<BigShot> gShots;

/* ========= Multi-click FSM ========= */
enum class MCState { IDLE, COUNTING };
struct MultiClick {
    MCState st{ MCState::IDLE };
    int count{ 0 }; int x0{ 0 }, y0{ 0 }; Uint32 silence{ 0 };
    void reset() { st = MCState::IDLE; count = 0; silence = 0; }
    bool sameSpot(int x, int y) const { return std::abs(x - x0) <= cfg::MULTI_TOL && std::abs(y - y0) <= cfg::MULTI_TOL; }
    void onMouseDown(int mx, int my) {
        if (st == MCState::IDLE) { st = MCState::COUNTING; count = 1; x0 = mx; y0 = my; silence = cfg::MULTI_SILENCE_MS; }
        else if (sameSpot(mx, my)) { ++count; silence = cfg::MULTI_SILENCE_MS; }
        else reset();
    }
    void onMotion(int mx, int my) { if (st == MCState::COUNTING && !sameSpot(mx, my)) reset(); }
    int onTick(Uint32 dt_ms) { if (st != MCState::COUNTING) return 0; if (dt_ms >= silence) { int n = count; reset(); return n; } silence -= dt_ms; return 0; }
};

/* ========= Ponto do cano da arma ========= */
static inline void compute_muzzle(float charX, int groundY, Facing face, int& outX, int& outY) {
    const int dstW = int(cfg::FRAME * cfg::SCALE);
    const int dstH = int(cfg::FRAME * cfg::SCALE);

    const float fx = (face == Facing::RIGHT ? cfg::MUZZLE_X_R_F : cfg::MUZZLE_X_L_F);
    const int   fx_px = (face == Facing::RIGHT ? cfg::MUZZLE_X_PX_R : cfg::MUZZLE_X_PX_L);

    outX = int(charX) + int(fx * dstW) + fx_px;
    outY = groundY + int(cfg::MUZZLE_Y_F * dstH) + cfg::MUZZLE_Y_PX;
}

/* ========= cria BigShot no cano e com poder ~ n ========= */
static void spawn_big_shot(int n, float charX, int groundY, Facing face) {
    n = std::max(1, std::min(n, 6));
    BigShot b;
    int mx, my; compute_muzzle(charX, groundY, face, mx, my);
    b.x = float(mx); b.y = float(my);
    b.vx = (face == Facing::RIGHT ? +cfg::BIGSHOT_BASE_SPEED : -cfg::BIGSHOT_BASE_SPEED);
    b.maxRange = cfg::BIGSHOT_BASE_RANGE + cfg::BIGSHOT_ADD_PER_N * (n - 1);
    b.scale = cfg::BIGSHOT_SCALE_MIN + cfg::BIGSHOT_SCALE_STEP * n;
    if (b.scale > 1.0f) b.scale = 1.0f;
    b.frame = 0; b.frame_ms = 40; b.alive = true;
    gShots.push_back(b);
    if (gShots.size() > 24) gShots.erase(gShots.begin());
}

int main(int, char**) {
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0);
    assert(IMG_Init(IMG_INIT_PNG) != 0);
    assert(TTF_Init() == 0);

    SDL_Window* win = SDL_CreateWindow("City4 + FSM + MultiClick BigShot",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cfg::WIN_W, cfg::WIN_H, SDL_WINDOW_SHOWN);
    assert(win);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(ren);

    SDL_Texture* bg = IMG_LoadTexture(ren, "City4.png");
    if (!bg) { std::printf("Nao foi possivel abrir City4.png: %s\n", IMG_GetError()); return 1; }
    SDL_Rect bgDst{ 0,0,cfg::WIN_W,cfg::WIN_H };

    // personagem
    std::vector<Anim> A(static_cast<size_t>(State::COUNT));
    auto S = [&](State s)->Anim& { return A[static_cast<size_t>(s)]; };
    load_anim(ren, &S(State::IDLE), "Idle.png", 120, true);
    load_anim(ren, &S(State::IDLE2), "Idle_2.png", 120, true);
    load_anim(ren, &S(State::WALK), "Walk.png", 90, true);
    load_anim(ren, &S(State::RUN), "Run.png", 70, true);
    load_anim(ren, &S(State::SHOT), "Shot.png", 70, false);
    load_anim(ren, &S(State::ATTACK), "Attack_1.png", 80, false);
    load_anim(ren, &S(State::JUMP), "Jump.png", 90, false);
    load_anim(ren, &S(State::HURT), "Hurt.png", 110, false);
    load_anim(ren, &S(State::RECHARGE), "Recharge.png", 100, false);
    load_anim(ren, &S(State::DEAD), "Dead.png", 140, false);

    // frames da explosão (para o projétil)
    for (int i = 0; i < 10; ++i) {
        char name[64]; std::snprintf(name, sizeof(name), "Explosion_%d.png", i + 1);
        gExpl[i] = IMG_LoadTexture(ren, name);
        if (!gExpl[i]) std::printf("Aviso: nao encontrei %s\n", name);
    }

    auto has = [&](State s) { return S(s).valid; };
    State locomotion = has(State::IDLE) ? State::IDLE : (has(State::WALK) ? State::WALK : State::RUN);
    State st = locomotion; int cur = 0;
    Uint32 frameLeft = S(st).valid ? S(st).frame_ms : 80;
    Facing face = Facing::RIGHT;

    bool kL = false, kR = false, kShift = false, locked = false, dead = false;
    float v = 0.f, x = cfg::WIN_W * 0.2f;
    const int groundY = cfg::WIN_H - int(cfg::FRAME * cfg::SCALE) - cfg::GROUND_OFFSET;

    // ajuda
    const char* tryFonts[] = { "tiny.ttf","C:/Windows/Fonts/consola.ttf","C:/Windows/Fonts/segoeui.ttf",nullptr };
    TTF_Font* font = nullptr; for (int i = 0; tryFonts[i]; ++i) { font = TTF_OpenFont(tryFonts[i], 18); if (font) break; }
    SDL_Color white{ 255,255,255,255 }, black{ 0,0,0,180 }; bool showHelp = true;
    std::vector<TextLine> help;
    if (font) {
        const char* lines[] = {
            "Controles:",
            "Setas <-/-> mover  |  Shift correr  |  ESC sair",
            "J: Attack_1 | K: Shot (sem explosao) | ESPACO: Jump | I: troca Idle",
            "Multi-clique (sem mover): BigShot explosivo mais potente",
            "F1: mostra/oculta painel",
            nullptr
        };
        for (int i = 0; lines[i]; ++i) help.push_back(make_text(ren, font, lines[i], white));
    }

    // multi-clique
    MultiClick mc{};
    Uint32 espera = cfg::MAX_WAIT_MS;
    Uint32 lastTicks = SDL_GetTicks();
    SDL_Event ev; bool running = true;

    while (running) {
        if (AUX_WaitEventTimeoutCount(&ev, &espera)) {
            if (ev.type == SDL_QUIT) running = false;

            if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                mc.onMouseDown(ev.button.x, ev.button.y);
            }
            else if (ev.type == SDL_MOUSEMOTION) {
                mc.onMotion(ev.motion.x, ev.motion.y);
            }
            else if (ev.type == SDL_KEYDOWN) {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_ESCAPE) running = false;
                if (k == SDLK_F1) showHelp = !showHelp;
                if (!dead) {
                    if (k == SDLK_LEFT) { kL = true;  face = Facing::LEFT; }
                    if (k == SDLK_RIGHT) { kR = true;  face = Facing::RIGHT; }
                    if (k == SDLK_LSHIFT || k == SDLK_RSHIFT) kShift = true;
                    if (!locked) {
                        if (k == SDLK_j && has(State::ATTACK)) { st = State::ATTACK;   locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_k && has(State::SHOT)) { st = State::SHOT;     locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_SPACE && has(State::JUMP)) { st = State::JUMP;     locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_h && has(State::HURT)) { st = State::HURT;     locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_r && has(State::RECHARGE)) { st = State::RECHARGE; locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_l && has(State::DEAD)) { st = State::DEAD; dead = true; locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                        if (k == SDLK_i && has(State::IDLE2)) {
                            State alt = State::IDLE2;
                            locomotion = (locomotion == alt && has(State::IDLE)) ? State::IDLE : alt;
                        }
                    }
                }
            }
            else if (ev.type == SDL_KEYUP) {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_LEFT)  kL = false;
                if (k == SDLK_RIGHT) kR = false;
                if (k == SDLK_LSHIFT || k == SDLK_RSHIFT) kShift = false;
            }
            else if (ev.type == SDL_USEREVENT) {
                // Final do multiclique → toca SHOT e cria BigShot no cano
                if (has(State::SHOT)) { st = State::SHOT; locked = true; cur = 0; frameLeft = S(st).frame_ms; }
                int n = ev.user.code; if (n < 1) n = 1; if (n > 6) n = 6;
                spawn_big_shot(n, x, groundY, face);
            }
        }
        else {
            // ===== Tick / simulação =====
            Uint32 now = SDL_GetTicks();
            Uint32 dt_ms = now - lastTicks;
            float  dt = dt_ms / 1000.f;
            lastTicks = now;
            if (dt > cfg::MAX_DT) dt = cfg::MAX_DT;

            // multiclique → quando encerra o silêncio, empurra USEREVENT
            if (int n = mc.onTick(dt_ms)) {
                SDL_Event u{}; u.type = SDL_USEREVENT; u.user.code = n; SDL_PushEvent(&u);
            }

            // animação do estado atual
            if (espera >= frameLeft) {
                if (S(st).loop) cur = (cur + 1) % std::max(1, S(st).cols);
                else if (cur < S(st).cols - 1) cur++;
                frameLeft = S(st).frame_ms;
                if (!S(st).loop && cur == S(st).cols - 1) {
                    if (st != State::DEAD) { locked = false; st = locomotion; cur = 0; frameLeft = S(st).frame_ms; }
                }
            }
            else frameLeft -= espera;

            // movimento horizontal do personagem
            float target = 0.f;
            if (!locked && !dead) {
                float maxspd = kShift ? cfg::RUN_SPD : cfg::WALK_SPD;
                if (kL ^ kR) target = (kL ? -maxspd : +maxspd);
            }
            if (target != 0.f) {
                float dv = target - v, step = cfg::ACCEL * dt;
                if (std::fabs(dv) <= step) v = target; else v += (dv > 0 ? step : -step);
            }
            else {
                if (v > 0) v = std::max(0.f, v - cfg::DECEL * dt);
                if (v < 0) v = std::min(0.f, v + cfg::DECEL * dt);
            }
            if (!locked && !dead) {
                State ns = (std::fabs(v) < 1.f) ? locomotion : (std::fabs(v) <= cfg::WALK_SPD + 1.f ? State::WALK : State::RUN);
                if (ns != st && S(ns).valid) { st = ns; cur = 0; frameLeft = S(st).frame_ms; }
            }
            x += v * dt;
            int maxX = cfg::WIN_W - int(cfg::FRAME * cfg::SCALE) - cfg::SIDE_MARGIN;
            if (x < cfg::SIDE_MARGIN) x = float(cfg::SIDE_MARGIN);
            if (x > maxX) x = float(maxX);

            // ===== Atualizar BigShots =====
            for (auto& b : gShots) if (b.alive) {
                b.x += b.vx * dt;
                b.traveled += std::fabs(b.vx * dt);
                // anima ciclo de chamas
                if (espera >= b.frame_ms) { b.frame = (b.frame + 1) % 10; b.frame_ms = 40; }
                else b.frame_ms -= espera;
                if (b.traveled >= b.maxRange || b.x < -300 || b.x > cfg::WIN_W + 300) b.alive = false;
            }
            gShots.erase(std::remove_if(gShots.begin(), gShots.end(),
                [](const BigShot& b) {return !b.alive; }), gShots.end());

            espera = (frameLeft < cfg::MAX_WAIT_MS ? frameLeft : cfg::MAX_WAIT_MS);
        }

        // ===== render =====
        SDL_RenderCopy(ren, bg, nullptr, &bgDst);

        // personagem
        const Anim* curAnim = &S(st);
        SDL_Rect src{ cur * cfg::FRAME, 0, cfg::FRAME, cfg::FRAME };
        SDL_Rect dst{ int(x), groundY, int(cfg::FRAME * cfg::SCALE), int(cfg::FRAME * cfg::SCALE) };
        SDL_RenderCopyEx(ren, curAnim->tex, &src, &dst, 0.0, nullptr,
            (face == Facing::LEFT ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));

        // BigShots (explosão “cometa”)
        for (const auto& b : gShots) if (b.alive && gExpl[b.frame]) {
            // sprites ~256x256; centraliza no b.x/b.y
            int w = 256, h = 256;
            SDL_Rect bd{ int(b.x - w * b.scale * 0.5f), int(b.y - h * b.scale * 0.5f),
                         int(w * b.scale), int(h * b.scale) };
            SDL_RenderCopy(ren, gExpl[b.frame], nullptr, &bd);
        }

        if (font && showHelp) draw_help(ren, help, black);
        SDL_RenderPresent(ren);
    }

    // cleanup
    for (auto& an : A) if (an.tex) SDL_DestroyTexture(an.tex);
    if (bg) SDL_DestroyTexture(bg);
    for (auto& t : gExpl) if (t) SDL_DestroyTexture(t);
    for (auto& tl : help) if (tl.tex) SDL_DestroyTexture(tl.tex);
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}
