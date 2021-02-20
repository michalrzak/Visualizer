#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include <cmath>

#include "cleanup.h"
#include "Function.h"

#define WIDTH  640
#define HEIGHT  480


std::ostream& logSDLError(std::ostream &os, const std::string &msg){
	return os << msg << " error: " << SDL_GetError() << '\n';
} 

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren){
	SDL_Texture *texture {nullptr};
	SDL_Surface *loaded_image {SDL_LoadBMP(file.c_str())};

	if(!loaded_image){
		logSDLError(std::cerr, "LoadBMP");
		return texture;
	}

	texture = SDL_CreateTextureFromSurface(ren, loaded_image);
	Util::cleanup(loaded_image);
	if(!texture)
		logSDLError(std::cerr, "CreateTextureFromSurface");

	return texture;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	
	SDL_QueryTexture(tex, nullptr, nullptr, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, nullptr, &dst);
}

int main(){
	if(SDL_Init(SDL_INIT_EVERYTHING)){
		logSDLError(std::cerr, "SDL_Init");
		return 1;
	}

	SDL_Window *win {SDL_CreateWindow("Lesson2", 500, 500, WIDTH, HEIGHT, SDL_WINDOW_SHOWN)};
	if(!win){
		logSDLError(std::cerr, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	
	SDL_Renderer *ren {SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)};
	if(!ren){
		logSDLError(std::cerr, "CreateRenderer");
		Util::cleanup(win);
		SDL_Quit();
		return 1;
	}
	
	bool done {false};
    Function f {Function(0.05)};
    double start {0};
	while (!done) {
        SDL_Event event;

        SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(ren);
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
        for (double x {0}; x < WIDTH; x+=0.5*2) {
            SDL_RenderDrawLine(ren, x, HEIGHT/2+f.next_sample()*75, x+0.5, HEIGHT/2+f.next_sample()*75);
        }
        
        SDL_RenderPresent(ren);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RIGHT)
                    start+=0.5;
                else if (event.key.keysym.sym == SDLK_LEFT)
                    start-=0.5;
            }
        }
        
        f.set_current_sample(start);
        //SDL_Delay(100);
    }
    
    Util::cleanup(ren, win);

    
	/*for (size_t i{0}; i < 3; ++i) {
		SDL_RenderPresent(ren);
		SDL_Delay(1000);
	}*/

	SDL_Quit();
	return 0;
}

