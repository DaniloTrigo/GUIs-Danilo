#include <SDL2/SDL.h>

typedef struct {
    SDL_Rect r;
    int cores[3];
} Rect;

int main(int argc, char* args[])
{
    /* INICIALIZACAO */
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* win = SDL_CreateWindow("Movendo um Retangulo e clicks geram retangulos",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        500, 500, SDL_WINDOW_SHOWN
    );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    /* EXECUÇÃO */
    SDL_Event evt;
    SDL_Rect r = { 40,20, 80,80};

    int executando = 1;
    int count = 0; //contador de retangulos
    Rect rects[10]; //array de retangulos

    while (executando) {
        SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF); 
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0xFF, 0xFF); 
        SDL_RenderFillRect(ren, &r);
        SDL_WaitEvent(&evt);

    for (int i = 0; i < count; i++) {
        SDL_SetRenderDrawColor(ren, rects[i].cores[0], rects[i].cores[1], rects[i].cores[2], 255);
	    SDL_RenderFillRect(ren, &rects[i].r);
    }

     SDL_RenderPresent(ren);

    if (evt.type == SDL_MOUSEBUTTONDOWN && count < 10) {
         int x, y;
		 SDL_GetMouseState(&x, &y);
         rects[count].r.x = x;
         rects[count].r.y = y;
         rects[count].r.w = 40;
         rects[count].r.h = 40;
         for (int j = 0; j < 3; j++) {
             rects[count].cores[j] = rand() % 255;
         }
     count++;
     }
     

     SDL_RenderPresent(ren);



        switch (evt.type) {
        case SDL_QUIT: // fechar janela
            executando = 0;
            break;

        case SDL_KEYDOWN:
            // ALT+F4 explícito
            if (evt.key.keysym.sym == SDLK_F4 && (evt.key.keysym.mod & KMOD_ALT)) {
                executando = 0;
                break;
            }

            switch (evt.key.keysym.sym) {
            case SDLK_UP:    
                r.y -= 5; 
                break;
            case SDLK_DOWN:  
                r.y += 5; 
                break;
            case SDLK_LEFT:  
                r.x -= 5; 
                break;
            case SDLK_RIGHT: 
                r.x += 5; 
                break;
            case SDLK_ESCAPE: 
                executando = 0; 
                break;
            }
            break;

        }

    }

    /* FINALIZACAO */
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
