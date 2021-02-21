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
	
	//mouse scrolling variables
	bool mouse_left_down {false};
	int initial_x;
    int initial_y;
    
    Function f {Function(0.005)};
    double offset_x {0};
    double offset_y {0};
    
    bool done {false};
	while (!done) {
        SDL_Event event;

        SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(ren);
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
        
        double prev = f.next_sample()*75;
        for (double x {0}; x < WIDTH; x+=0.05*2) {
            double next = f.next_sample()*75;
            SDL_RenderDrawLine(ren, x, HEIGHT/2 - prev + offset_y, x+0.5, HEIGHT/2 - next + offset_y);
            prev = next;
        }
        
        
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouse_left_down = true;
                        initial_x = event.button.x;
                        initial_y = event.button.y;
                    }
                    
                    break;
                
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        mouse_left_down = false;
                    break;
            }
        }
        
        if (mouse_left_down) {
            int current_x;
            int current_y;
            SDL_GetMouseState(&current_x, &current_y);
            
            SDL_RenderDrawLine(ren, current_x, current_y-10, current_x, current_y+10);
            SDL_RenderDrawLine(ren, current_x-10, current_y, current_x+10, current_y);
            
            offset_x += (initial_x-current_x)/4.0;
            offset_y -= (initial_y-current_y)*1.5;
            
            initial_x = current_x;
            initial_y = current_y;
        }
        
        SDL_RenderPresent(ren);
        
        f.set_current_sample(offset_x);
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

