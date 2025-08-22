#include <SDL2/SDL.h>

int main(int argc, char* args[])
{
    /* INICIALIZACAO */
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* win = SDL_CreateWindow("Gol (noite com estrelas)",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        500, 500, SDL_WINDOW_SHOWN
    );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    /* EXECUÇÃO */
    // Fundo azul escuro
    SDL_SetRenderDrawColor(ren, 10, 14, 28, 255);
    SDL_RenderClear(ren);

    // Topo esquerdo
    SDL_SetRenderDrawColor(ren, 255, 255, 245, 255); 
    SDL_RenderDrawPoint(ren, 15, 25);
    SDL_RenderDrawPoint(ren, 30, 40);
    SDL_RenderDrawPoint(ren, 50, 20);
    SDL_RenderDrawPoint(ren, 70, 60);
    SDL_RenderDrawPoint(ren, 85, 35);
    SDL_RenderDrawPoint(ren, 60, 90);
    SDL_RenderDrawPoint(ren, 40, 75);
    SDL_RenderDrawPoint(ren, 25, 55);
    SDL_RenderDrawPoint(ren, 80, 15);
    SDL_RenderDrawPoint(ren, 90, 100);

    // Topo direito
    SDL_RenderDrawPoint(ren, 415, 30);
    SDL_RenderDrawPoint(ren, 430, 50);
    SDL_RenderDrawPoint(ren, 460, 25);
    SDL_RenderDrawPoint(ren, 480, 40);
    SDL_RenderDrawPoint(ren, 495, 10);
    SDL_RenderDrawPoint(ren, 440, 80);
    SDL_RenderDrawPoint(ren, 420, 100);
    SDL_RenderDrawPoint(ren, 470, 90);
    SDL_RenderDrawPoint(ren, 488, 70);
    SDL_RenderDrawPoint(ren, 430, 20);

    // Topo central
    SDL_RenderDrawPoint(ren, 150, 50);
    SDL_RenderDrawPoint(ren, 170, 30);
    SDL_RenderDrawPoint(ren, 200, 40);
    SDL_RenderDrawPoint(ren, 250, 20);
    SDL_RenderDrawPoint(ren, 300, 35);
    SDL_RenderDrawPoint(ren, 350, 55);
    SDL_RenderDrawPoint(ren, 220, 80);
    SDL_RenderDrawPoint(ren, 280, 90);
    SDL_RenderDrawPoint(ren, 320, 70);
    SDL_RenderDrawPoint(ren, 180, 100);

    // gramado (parte inferior)
    SDL_SetRenderDrawColor(ren, 34, 139, 34, 255);
    SDL_Rect gramado = { 0, 380, 500, 120 };
    SDL_RenderFillRect(ren, &gramado);

    // linha limite do gramado
    SDL_SetRenderDrawColor(ren, 238, 238, 238, 255);
    SDL_RenderDrawLine(ren, 0, 380, 500, 380);


    // REDE 
    SDL_SetRenderDrawColor(ren, 210, 210, 210, 255);

    // verticais
    SDL_RenderDrawLine(ren, 104, 124, 104, 376);
    SDL_RenderDrawLine(ren, 128, 124, 128, 376);
    SDL_RenderDrawLine(ren, 152, 124, 152, 376);
    SDL_RenderDrawLine(ren, 176, 124, 176, 376);
    SDL_RenderDrawLine(ren, 200, 124, 200, 376);
    SDL_RenderDrawLine(ren, 224, 124, 224, 376);
    SDL_RenderDrawLine(ren, 248, 124, 248, 376);
    SDL_RenderDrawLine(ren, 272, 124, 272, 376);
    SDL_RenderDrawLine(ren, 296, 124, 296, 376);
    SDL_RenderDrawLine(ren, 320, 124, 320, 376);
    SDL_RenderDrawLine(ren, 344, 124, 344, 376);
    SDL_RenderDrawLine(ren, 368, 124, 368, 376);
    SDL_RenderDrawLine(ren, 392, 124, 392, 376);
    // horizontais
    SDL_RenderDrawLine(ren, 104, 132, 396, 132);
    SDL_RenderDrawLine(ren, 104, 156, 396, 156);
    SDL_RenderDrawLine(ren, 104, 180, 396, 180);
    SDL_RenderDrawLine(ren, 104, 204, 396, 204);
    SDL_RenderDrawLine(ren, 104, 228, 396, 228);
    SDL_RenderDrawLine(ren, 104, 252, 396, 252);
    SDL_RenderDrawLine(ren, 104, 276, 396, 276);
    SDL_RenderDrawLine(ren, 104, 300, 396, 300);
    SDL_RenderDrawLine(ren, 104, 324, 396, 324);
    SDL_RenderDrawLine(ren, 104, 348, 396, 348);
    SDL_RenderDrawLine(ren, 104, 372, 396, 372);

    // TRAVE (postes + travessão)
    SDL_SetRenderDrawColor(ren, 245, 245, 245, 255);
    SDL_Rect posteL = { 88, 120, 12, 260 };
    SDL_RenderFillRect(ren, &posteL);
    SDL_Rect posteR = { 400, 120, 12, 260 };
    SDL_RenderFillRect(ren, &posteR);
    SDL_Rect trav = { 88, 108, 324, 12 };
    SDL_RenderFillRect(ren, &trav);

    // linha do gol
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderDrawLine(ren, 100, 380, 400, 380);

    // “bola”
    SDL_Rect bola = { 246, 366, 20, 20 };
    SDL_RenderFillRect(ren, &bola);

    SDL_RenderPresent(ren);
    SDL_Delay(5000);

    /* FINALIZACAO */
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
