#include <SDL2/SDL.h>

/* -------- Função auxiliar -------- */
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms) {
    if (!ms) return 0;
    Uint32 antes = SDL_GetTicks();
    int ret = SDL_WaitEventTimeout(evt, *ms);
    Uint32 decorrido = SDL_GetTicks() - antes;
    if (*ms > decorrido) *ms -= decorrido;
    else *ms = 0;
    return ret;
}

int main(int argc, char* args[])
{
    /* INICIALIZAÇÃO */
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 1;
    SDL_Window* win = SDL_CreateWindow("Animando e Movendo um Retângulo",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        200, 100, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    /* EXECUÇÃO */
    SDL_Rect r = { 40,20, 10,10 };
    Uint32 espera = 500;          
    int running = 1;

    while (running) {
        // desenho
        SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);  
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0xFF, 0xFF);
        SDL_RenderFillRect(ren, &r);
        SDL_RenderPresent(ren);

        // espera por evento com contagem regressiva
        SDL_Event evt;
        int isevt = AUX_WaitEventTimeoutCount(&evt, &espera);

        if (isevt) {
            if (evt.type == SDL_QUIT) {
                running = 0;
            }
            else if (evt.type == SDL_KEYDOWN) {
                switch (evt.key.keysym.sym) {
                case SDLK_ESCAPE: running = 0; break;
                case SDLK_UP:    
                    r.y -= 5; break;
                case SDLK_DOWN:  
                    r.y += 5; break;
                case SDLK_LEFT: 
                    r.x -= 5; break;
                case SDLK_RIGHT: 
                    r.x += 5; break;
                }
            }
            
        }
        else {
            
            r.x += 2;
            r.y += 2;
            espera = 500;          
        }
    }

    /* FINALIZAÇÃO */
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
