#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

/* === Auxiliar (sua função) === */
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms) {
    Uint32 antes = SDL_GetTicks();
    Uint32 depois = 0;
    int isevt = SDL_WaitEventTimeout(evt, *ms);
    if (isevt) {
        depois = (SDL_GetTicks() - antes);
        if (*ms < depois) depois = *ms;
        *ms -= depois;
    }
    else {
        *ms = 500;
    }
    return isevt;
}

/* Clamp na linha de chegada e marca finalização no primeiro toque */
static inline bool clamp_and_finish(SDL_Rect* outer, SDL_Rect* inner, int FINISH_X) {
    if (outer->x + outer->w >= FINISH_X) {
        outer->x = FINISH_X - outer->w;
        if (inner) inner->x = outer->x + 2;
        return true; /* cruzou/encostou na linha nesta atualização */
    }
    return false;
}

/* Faixa quadriculada ESTÁTICA em x=260..299 (sem compound literals) */
static void draw_finish_line_static(SDL_Renderer* ren, int height) {
    SDL_Rect cell;
    cell.w = 10;
    cell.h = 10;
    for (int yrow = 0; yrow < height; yrow += 10) {
        int odd = ((yrow / 10) & 1);
        cell.y = yrow;

        /* coluna 260 */
        cell.x = 260;
        SDL_SetRenderDrawColor(ren, odd ? 255 : 0, odd ? 255 : 0, odd ? 255 : 0, 255);
        SDL_RenderFillRect(ren, &cell);

        /* coluna 270 */
        cell.x = 270;
        SDL_SetRenderDrawColor(ren, odd ? 0 : 255, odd ? 0 : 255, odd ? 0 : 255, 255);
        SDL_RenderFillRect(ren, &cell);

        /* coluna 280 */
        cell.x = 280;
        SDL_SetRenderDrawColor(ren, odd ? 255 : 0, odd ? 255 : 0, odd ? 255 : 0, 255);
        SDL_RenderFillRect(ren, &cell);

        /* coluna 290 */
        cell.x = 290;
        SDL_SetRenderDrawColor(ren, odd ? 0 : 255, odd ? 0 : 255, odd ? 0 : 255, 255);
        SDL_RenderFillRect(ren, &cell);
    }
}

int main(int argc, char* args[])
{
    /* INICIALIZACAO */
    SDL_Init(SDL_INIT_EVERYTHING);
    const int LARG = 300, ALT = 300;
    SDL_Window* win = SDL_CreateWindow("corrida entre retangulos",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        LARG, ALT, SDL_WINDOW_SHOWN
    );
    /* Sem VSYNC (flags = 0). Se quiser GPU sem vsync: SDL_RENDERER_ACCELERATED */
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    const int FINISH_X = 280; /* borda direita trava aqui */

    int continua = 1, telacorrida = 1, telafinal = 0;
    SDL_Rect r1 = { 10,  40, 20,20 };
    SDL_Rect r2 = { 10, 120, 20,20 };
    SDL_Rect r3 = { 10, 210, 20,20 };

    int x, y;
    SDL_Rect r11 = { 12,  42, 16,16 };  /* miolos coloridos */
    SDL_Rect r22 = { 12, 122, 16,16 };
    SDL_Rect r33 = { 12, 212, 16,16 };

    Uint32 espera = 500;
    int vencedor = 0, segundolugar = 0, terceirolugar = 0;

    bool finished[3] = { false,false,false }; /* r1, r2, r3 */
    int  finishCount = 0;

    /* EXECUÇÃO */
    while (continua) {
        while (telacorrida) {
            /* Fundo (grama) */
            SDL_SetRenderDrawColor(ren, 0x55, 0x6B, 0x2F, 255);
            SDL_RenderClear(ren);

            /* Linha de chegada estática */
            draw_finish_line_static(ren, ALT);

            /* Carros: moldura preta + miolo colorido */
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderFillRect(ren, &r1);
            SDL_RenderFillRect(ren, &r2);
            SDL_RenderFillRect(ren, &r3);

            SDL_SetRenderDrawColor(ren, 0x93, 0x70, 0xDB, 255); SDL_RenderFillRect(ren, &r22); /* roxo (r2) */
            SDL_SetRenderDrawColor(ren, 255, 0, 255, 255);     SDL_RenderFillRect(ren, &r11); /* magenta (r1) */
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);   SDL_RenderFillRect(ren, &r33); /* branco  (r3) */

            SDL_RenderPresent(ren);

            SDL_Event evt;
            if (AUX_WaitEventTimeoutCount(&evt, &espera)) {
                if (evt.type == SDL_QUIT) { continua = 0; break; }

                switch (evt.type) {
                case SDL_MOUSEMOTION:
                    /* r3 controlado pelo mouse: apenas X (clamp 0..FINISH_X - w) */
                    if (!finished[2]) {
                        SDL_GetMouseState(&x, &y);
                        if (x < 0) x = 0;
                        if (x > FINISH_X - r3.w) x = FINISH_X - r3.w;
                        r3.x = x;
                        r33.x = r3.x + 2;

                        if (clamp_and_finish(&r3, &r33, FINISH_X) && !finished[2]) {
                            finished[2] = true; finishCount++;
                            if (!vencedor) vencedor = 3;
                            else if (!segundolugar) segundolugar = 3;
                            else if (!terceirolugar) terceirolugar = 3;
                        }
                    }
                    break;

                case SDL_KEYDOWN:
                    if (evt.key.keysym.sym == SDLK_ESCAPE) { continua = 0; break; }
                    if (evt.key.keysym.sym == SDLK_RIGHT && !finished[0]) {
                        /* r1: só X */
                        r1.x += 2;
                        r11.x += 2;

                        if (clamp_and_finish(&r1, &r11, FINISH_X) && !finished[0]) {
                            finished[0] = true; finishCount++;
                            if (!vencedor) vencedor = 1;
                            else if (!segundolugar) segundolugar = 1;
                            else if (!terceirolugar) terceirolugar = 1;
                        }
                    }
                    break;
                }
            }
            else {
                /* timeout: r2 anda sozinho (só X) */
                espera = 100;
                if (!finished[1]) {
                    r2.x += 3;
                    r22.x += 3;

                    if (clamp_and_finish(&r2, &r22, FINISH_X) && !finished[1]) {
                        finished[1] = true; finishCount++;
                        if (!vencedor) vencedor = 2;
                        else if (!segundolugar) segundolugar = 2;
                        else if (!terceirolugar) terceirolugar = 2;
                    }
                }
            }

            /* Todos terminaram? */
            if (finishCount == 3) {
                telacorrida = 0;
                telafinal = 1;
            }
        }

        /* Tela final */
        while (telafinal) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderClear(ren);

            stringRGBA(ren, 50, 50, "Resultado da Corrida:", 255, 255, 255, 255);
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "1º Lugar: Carro %d", vencedor);
            stringRGBA(ren, 50, 100, buffer, 255, 255, 255, 255);
            snprintf(buffer, sizeof(buffer), "2º Lugar: Carro %d", segundolugar);
            stringRGBA(ren, 50, 150, buffer, 255, 255, 255, 255);
            snprintf(buffer, sizeof(buffer), "3º Lugar: Carro %d", terceirolugar);
            stringRGBA(ren, 50, 200, buffer, 255, 255, 255, 255);

            stringRGBA(ren, 20, 250, "ESC: sair | R: reiniciar", 255, 255, 255, 255);
            SDL_RenderPresent(ren);

            SDL_Event evt;
            if (SDL_WaitEvent(&evt)) {
                if (evt.type == SDL_QUIT) { telafinal = 0; continua = 0; }
                else if (evt.type == SDL_KEYDOWN) {
                    if (evt.key.keysym.sym == SDLK_ESCAPE) { telafinal = 0; continua = 0; }
                    if (evt.key.keysym.sym == SDLK_r) {
                        /* Reinicia corrida */
                        telafinal = 0; telacorrida = 1; continua = 1;
                        r1.x = r2.x = r3.x = 10;
                        r11.x = r22.x = r33.x = 12;
                        vencedor = segundolugar = terceirolugar = 0;
                        finished[0] = finished[1] = finished[2] = false;
                        finishCount = 0;
                        espera = 500;
                        break;
                    }
                }
            }
        }
    }

    /* FINALIZACAO */
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
