//#include <SDL2/SDL.h>
//#include <SDL2/SDL2_gfxPrimitives.h>
//
//int main(int argc, char** argv) {
//    SDL_Init(SDL_INIT_EVERYTHING);
//    SDL_Window* win = SDL_CreateWindow("Vegeta!!!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_SHOWN);
//    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
//
//    int x = 0;            // deslocamento horizontal
//    int dx = 10;          // velocidade
//    int running = 1;
//   
//
//    while (running) {
//        // Limpa a tela
//        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
//        SDL_RenderClear(ren);
//
//        // fundo branco
//        boxRGBA(ren, 0, 0, 500, 500, 255, 255, 255, 255);
//        // chão
//        boxRGBA(ren, 0, 460, 500, 500, 0, 0, 0, 255);
//
//        // --------- CABELO ---------
//        boxRGBA(ren, 180 + x, 110, 320 + x, 130, 10, 10, 10, 255);
//        boxRGBA(ren, 190 + x, 90, 310 + x, 110, 10, 10, 10, 255);
//        boxRGBA(ren, 200 + x, 70, 300 + x, 90, 10, 10, 10, 255);
//        boxRGBA(ren, 210 + x, 55, 290 + x, 70, 10, 10, 10, 255);
//        boxRGBA(ren, 230 + x, 40, 270 + x, 55, 10, 10, 10, 255);
//        boxRGBA(ren, 245 + x, 25, 255 + x, 40, 10, 10, 10, 255);
//        boxRGBA(ren, 165 + x, 100, 185 + x, 115, 10, 10, 10, 255);
//        boxRGBA(ren, 155 + x, 80, 175 + x, 95, 10, 10, 10, 255);
//        boxRGBA(ren, 170 + x, 65, 190 + x, 80, 10, 10, 10, 255);
//        boxRGBA(ren, 315 + x, 100, 335 + x, 115, 10, 10, 10, 255);
//        boxRGBA(ren, 325 + x, 80, 345 + x, 95, 10, 10, 10, 255);
//        boxRGBA(ren, 310 + x, 65, 330 + x, 80, 10, 10, 10, 255);
//
//        // --------- ROSTO ---------
//        boxRGBA(ren, 190 + x, 110, 310 + x, 210, 255, 198, 141, 255);
//        boxRGBA(ren, 175 + x, 145, 190 + x, 185, 230, 174, 124, 255);
//        boxRGBA(ren, 310 + x, 145, 325 + x, 185, 230, 174, 124, 255);
//        boxRGBA(ren, 205 + x, 140, 240 + x, 150, 10, 10, 10, 255);
//        boxRGBA(ren, 260 + x, 140, 295 + x, 150, 10, 10, 10, 255);
//        boxRGBA(ren, 208 + x, 152, 238 + x, 163, 255, 255, 255, 255);
//        boxRGBA(ren, 268 + x, 152, 292 + x, 163, 255, 255, 255, 255);
//        boxRGBA(ren, 224 + x, 154, 229 + x, 160, 0, 0, 0, 255);
//        boxRGBA(ren, 278 + x, 154, 283 + x, 160, 0, 0, 0, 255);
//        boxRGBA(ren, 247 + x, 165, 253 + x, 178, 194, 140, 96, 255);
//        boxRGBA(ren, 232 + x, 186, 268 + x, 193, 120, 45, 45, 255);
//        boxRGBA(ren, 292 + x, 148, 318 + x, 168, 50, 180, 90, 255);
//        boxRGBA(ren, 318 + x, 148, 325 + x, 168, 120, 120, 120, 255);
//        boxRGBA(ren, 175 + x, 156, 205 + x, 160, 80, 80, 80, 255);
//        boxRGBA(ren, 205 + x, 155, 235 + x, 159, 80, 80, 80, 255);
//        boxRGBA(ren, 235 + x, 154, 262 + x, 158, 80, 80, 80, 255);
//        boxRGBA(ren, 262 + x, 153, 292 + x, 157, 80, 80, 80, 255);
//
//        // --------- TRONCO / ARMADURA ---------
//        boxRGBA(ren, 170 + x, 210, 330 + x, 350, 25, 40, 140, 255);
//        boxRGBA(ren, 180 + x, 220, 320 + x, 300, 245, 245, 245, 255);
//        boxRGBA(ren, 170 + x, 220, 195 + x, 235, 220, 180, 40, 255);
//        boxRGBA(ren, 170 + x, 235, 195 + x, 260, 220, 180, 40, 255);
//        boxRGBA(ren, 170 + x, 260, 195 + x, 285, 220, 180, 40, 255);
//        boxRGBA(ren, 170 + x, 285, 195 + x, 300, 220, 180, 40, 255);
//        boxRGBA(ren, 305 + x, 220, 330 + x, 235, 220, 180, 40, 255);
//        boxRGBA(ren, 305 + x, 235, 330 + x, 260, 220, 180, 40, 255);
//        boxRGBA(ren, 305 + x, 260, 330 + x, 285, 220, 180, 40, 255);
//        boxRGBA(ren, 305 + x, 285, 330 + x, 300, 220, 180, 40, 255);
//        boxRGBA(ren, 205 + x, 300, 295 + x, 312, 220, 180, 40, 255);
//        boxRGBA(ren, 205 + x, 314, 295 + x, 326, 220, 180, 40, 255);
//        boxRGBA(ren, 130 + x, 220, 170 + x, 260, 25, 40, 140, 255);
//        boxRGBA(ren, 330 + x, 220, 370 + x, 260, 25, 40, 140, 255);
//        boxRGBA(ren, 120 + x, 260, 170 + x, 300, 10, 10, 10, 255);
//        boxRGBA(ren, 330 + x, 260, 380 + x, 300, 10, 10, 10, 255);
//
//        // --------- PERNAS / BOTAS ---------
//        boxRGBA(ren, 200 + x, 350, 300 + x, 430, 25, 40, 140, 255);
//        // botas brancas
//        boxRGBA(ren, 180 + x, 430, 230 + x, 460, 245, 245, 245, 255);
//        boxRGBA(ren, 270 + x, 430, 320 + x, 460, 245, 245, 245, 255);
//        // bico dourado
//        boxRGBA(ren, 180 + x, 450, 230 + x, 460, 220, 180, 40, 255);
//        boxRGBA(ren, 270 + x, 450, 320 + x, 460, 220, 180, 40, 255);
//
//        // Apresenta a renderização
//        SDL_RenderPresent(ren);
//
//        // Delay para ~20 FPS
//        SDL_Delay(50);
//
//        // Atualiza posição horizontal
//        x += dx;
//
//        // Inverte a direção se atinge as bordas
//        if (x <= -100 || x >= 100) {
//            dx = -dx;
//        }
//    }
//
//    SDL_DestroyRenderer(ren);
//    SDL_DestroyWindow(win);
//    SDL_Quit();
//
//    return 0;
//}