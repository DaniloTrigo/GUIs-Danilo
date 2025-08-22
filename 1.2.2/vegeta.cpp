#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* win = SDL_CreateWindow("Vegeta!!!",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        500, 500, SDL_WINDOW_SHOWN
    );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

	boxRGBA(ren, 0, 0, 500, 500, 255, 255, 255, 255); // fundo branco

	boxRGBA(ren, 0, 500, 500, 460, 0, 0, 0, 255); // chão

	// --------- CABELO ---------
	// massa central superior
	boxRGBA(ren, 180, 110, 320, 130, 10, 10, 10, 255);
    boxRGBA(ren, 190, 90, 310, 110, 10, 10, 10, 255);
    boxRGBA(ren, 200, 70, 300, 90, 10, 10, 10, 255);
	boxRGBA(ren, 210, 55, 290, 70, 10, 10, 10, 255);
	boxRGBA(ren, 230, 40, 270, 55, 10, 10, 10, 255);
	boxRGBA(ren, 245, 25, 255, 40, 10, 10, 10, 255); // pico central

	// espinhos laterais (esq)
	boxRGBA(ren, 165, 100, 185, 115, 10, 10, 10, 255);
	boxRGBA(ren, 155, 80, 175, 95, 10, 10, 10, 255);
	boxRGBA(ren, 170, 65, 190, 80, 10, 10, 10, 255); 

    // espinhos laterais (dir)
    boxRGBA(ren, 315, 100, 335, 115, 10, 10, 10, 255);
    boxRGBA(ren, 325, 80, 345, 95, 10, 10, 10, 255);
    boxRGBA(ren, 310, 65, 330, 80, 10, 10, 10, 255);

    // --------- ROSTO ---------
    // face
    boxRGBA(ren, 190, 110, 310, 210, 255, 198, 141, 255);
    // orelhas / sombra lateral
    boxRGBA(ren, 175, 145, 190, 185, 230, 174, 124, 255);
    boxRGBA(ren, 310, 145, 325, 185, 230, 174, 124, 255);

    // sobrancelhas
    boxRGBA(ren, 205, 140, 240, 150, 10, 10, 10, 255);
    boxRGBA(ren, 260, 140, 295, 150, 10, 10, 10, 255);

    // olhos (branco + pupila)
    boxRGBA(ren, 208, 152, 238, 163, 255, 255, 255, 255);
    boxRGBA(ren, 268, 152, 292, 163, 255, 255, 255, 255);
    boxRGBA(ren, 224, 154, 229, 160, 0, 0, 0, 255);  // pupila esq
    boxRGBA(ren, 278, 154, 283, 160, 0, 0, 0, 255);  // pupila dir

    // nariz e boca
    boxRGBA(ren, 247, 165, 253, 178, 194, 140, 96, 255);
    boxRGBA(ren, 232, 186, 268, 193, 120, 45, 45, 255);

    // scouter 
    boxRGBA(ren, 292, 148, 318, 168, 50, 180, 90, 255);    // lente verde
    boxRGBA(ren, 318, 148, 325, 168, 120, 120, 120, 255);  // módulo lateral
    // scouter até a orelha 
    boxRGBA(ren, 175, 156, 205, 160, 80, 80, 80, 255);
    boxRGBA(ren, 205, 155, 235, 159, 80, 80, 80, 255);
    boxRGBA(ren, 235, 154, 262, 158, 80, 80, 80, 255);
    boxRGBA(ren, 262, 153, 292, 157, 80, 80, 80, 255);

    // --------- TRONCO / ARMADURA ---------
    // macacão azul
    boxRGBA(ren, 170, 210, 330, 350, 25, 40, 140, 255);

    // peitoral branco
    boxRGBA(ren, 180, 220, 320, 300, 245, 245, 245, 255);

    // ombreira esquerda (com degraus)
    boxRGBA(ren, 170, 220, 195, 235, 220, 180, 40, 255);
    boxRGBA(ren, 170, 235, 195, 260, 220, 180, 40, 255);
    boxRGBA(ren, 170, 260, 195, 285, 220, 180, 40, 255);
    boxRGBA(ren, 170, 285, 195, 300, 220, 180, 40, 255);

    // ombreira direita
    boxRGBA(ren, 305, 220, 330, 235, 220, 180, 40, 255);
    boxRGBA(ren, 305, 235, 330, 260, 220, 180, 40, 255);
    boxRGBA(ren, 305, 260, 330, 285, 220, 180, 40, 255);
    boxRGBA(ren, 305, 285, 330, 300, 220, 180, 40, 255);

    // placas abdominais (amarelas)
    boxRGBA(ren, 205, 300, 295, 312, 220, 180, 40, 255);
    boxRGBA(ren, 205, 314, 295, 326, 220, 180, 40, 255);

    // braços + luvas
    boxRGBA(ren, 130, 220, 170, 260, 25, 40, 140, 255);   // braço sup esq (azul)
    boxRGBA(ren, 330, 220, 370, 260, 25, 40, 140, 255);   // braço sup dir
    boxRGBA(ren, 120, 260, 170, 300, 10, 10, 10, 255); // luva esq
    boxRGBA(ren, 330, 260, 380, 300, 10, 10, 10, 255); // luva dir

    // --------- PERNAS / BOTAS ---------
    // pernas (azul)
    boxRGBA(ren, 200, 350, 300, 430, 25, 40, 140, 255);

    // botas brancas
    boxRGBA(ren, 180, 430, 230, 460, 245, 245, 245, 255);
    boxRGBA(ren, 270, 430, 320, 460, 245, 245, 245, 255);
    // bico dourado
    boxRGBA(ren, 180, 450, 230, 460, 220, 180, 40, 255);
    boxRGBA(ren, 270, 450, 320, 460, 220, 180, 40, 255);

    SDL_RenderPresent(ren);
 
    
     
    SDL_Delay(5000);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
