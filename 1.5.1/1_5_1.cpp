#include <assert.h>
#include <SDL2/SDL.h>

int main(int argc, char* args[])
{
    /* INICIALIZAÇÃO */
    SDL_Init(SDL_INIT_EVERYTHING);
    const int largura = 300;
    const int altura = 200;
    SDL_Window* win = SDL_CreateWindow("Contando o Tempo",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        largura, altura, SDL_WINDOW_SHOWN
    );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    /* EXECUÇÃO */
    SDL_Rect r = { 40, 20, 10, 10 };
    SDL_Rect r2 = { 80, 40, 20, 10 };
    SDL_Rect r3 = { 50, 50, 30, 15 };
    int espera = 500;

    int quit = 0;
    SDL_Event evt;

    while (!quit) {

        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) quit = 1;
        }


        SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0x00);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0xFF, 0x00);
        SDL_RenderFillRect(ren, &r);
        SDL_RenderFillRect(ren, &r2);
        SDL_RenderFillRect(ren, &r3);
        SDL_RenderPresent(ren);

        Uint32 antes = SDL_GetTicks();
        int isevt = SDL_WaitEventTimeout(&evt, espera);

        if (isevt) {
            espera -= (SDL_GetTicks() - antes);
            if (espera < 0) espera = 0;

            if (evt.type == SDL_KEYDOWN) {
                switch (evt.key.keysym.sym) {
                case SDLK_UP:
                    if (r.y > 0) 
                        r.y -= 5;
                    break;
                case SDLK_DOWN:
                    if (r.y + r2.h < altura) 
                        r.y += 5;
                    break;
                case SDLK_LEFT:
                    if (r.x > 0) 
                        r.x -= 5;
                    break;
                case SDLK_RIGHT:
                    if (r.x + r2.w < largura) 
                        r.x += 5;
                    break;
                }
            }

            if (evt.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                r2.x = x;
                r2.y = y;
            }

        }
        else {
            espera = 500;

         
            int novaX = r3.x + 2;
            int novaY = r3.y + 2;

            if (novaX + r3.w <= largura) r3.x = novaX;
            if (novaY + r3.h <= altura) r3.y = novaY;
        }
    }

    /* FINALIZAÇÃO */
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
