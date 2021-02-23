#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include <cmath>

#include "cleanup.h"
#include "Function.h"

//Change these numbers to alter the window size
#define WIDTH  640
#define HEIGHT  480

//Change these numbers to change the scrollspeed and direction
#define X_SCROLLSPEED 2.0
#define Y_SCROLLSPEED -2.0

//Change these number to change the zoom parameters
#define ZOOMSPEED 1.5
#define MIN_ZOOM 0.001
#define MAX_ZOOM 1000
#define CROSSHAIR_FADE_DURATION 20u

//Change these to alter the graph
#define SAMPLE_FREQUENCY 0.001
#define X_AXIS_SCALE 75
#define Y_AXIS_SCALE 75
#define HIGHLIGHT_INTERPOLANTS true

std::ostream& logSDLError(std::ostream &os, const std::string &msg){
	return os << msg << " error: " << SDL_GetError() << '\n';
} 


//This function creates a crosshair at the mouse position. It can also return the current mouse position.
void crosshair_at_mousepointer(SDL_Renderer* ren, int* mouse_x = nullptr, int* mouse_y = nullptr) {
    int current_x;
    int current_y;
    SDL_GetMouseState(&current_x, &current_y);
    
    SDL_RenderDrawLine(ren, current_x, current_y-10, current_x, current_y+10);
    SDL_RenderDrawLine(ren, current_x-10, current_y, current_x+10, current_y);
    
    if (mouse_x != nullptr)
        *mouse_x = current_x;
    if (mouse_y != nullptr)
        *mouse_y = current_y;
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
    
    //mouse zoom variables
    double zoom {1};
    unsigned int fade_crosshair {0};
    
    //Function variables
    Function f {Function(SAMPLE_FREQUENCY)};
    double offset_x {0};
    double offset_y {0};
    
    bool done {false};
    while (!done) {
        SDL_Event event;

        SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(ren);
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
        bool black {true};
        
        double prev = f.next_sample()*Y_AXIS_SCALE*zoom;
        for (double x {0}; x < WIDTH; x+=SAMPLE_FREQUENCY*X_AXIS_SCALE*zoom) {
            double next = f.next_sample()*Y_AXIS_SCALE*zoom;
            //The y coordinate has the default position in the middle of the screen
            SDL_RenderDrawLine(ren, x, /*HEIGHT/2 +*/ offset_y - prev , x+SAMPLE_FREQUENCY*X_AXIS_SCALE*zoom, /*HEIGHT/2 +*/ offset_y - next);
            prev = next;
            
            if (HIGHLIGHT_INTERPOLANTS) {
                if (black)
                    SDL_SetRenderDrawColor(ren, 255, 0, 0, SDL_ALPHA_OPAQUE);
                else
                    SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
                black = !black;
            }
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
                    
                case SDL_MOUSEWHEEL:
                    
                    //retrieve current mouse position to adjust offsets accordingly
                    //offsets are calculated from the [0;0] coordinate in the graph. 
                    //there may be a better way to do all this but this is functional
                    int current_x;
                    int current_y;
                    SDL_GetMouseState(&current_x, &current_y);
                                        
                    double cx {static_cast<double>(current_x+offset_x)};
                    double cy {static_cast<double>(-current_y+offset_y)};

                    if (event.wheel.y > 0 && zoom != MAX_ZOOM) {
                        zoom*=ZOOMSPEED;
                        if (zoom > MAX_ZOOM)
                            zoom = MAX_ZOOM;
                        offset_x += (cx*ZOOMSPEED-cx);
                        offset_y += (cy*ZOOMSPEED-cy);
                    }
                    
                    if (event.wheel.y < 0 && zoom != MIN_ZOOM) {
                        zoom/=ZOOMSPEED;
                        if (zoom < MIN_ZOOM)
                            zoom = MIN_ZOOM;
                        offset_x += (cx/ZOOMSPEED-cx);
                        offset_y += (cy/ZOOMSPEED-cy);
                    }
                    
                    fade_crosshair = CROSSHAIR_FADE_DURATION;
                                        
                    break;
                    
            }
        }
        
        if (mouse_left_down) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
            
            int current_x;
            int current_y;
            crosshair_at_mousepointer(ren, &current_x, &current_y);
            
            offset_x += (initial_x-current_x)*X_SCROLLSPEED;
            offset_y += (initial_y-current_y)*Y_SCROLLSPEED;
            
            initial_x = current_x;
            initial_y = current_y;
        }
        
        if (fade_crosshair) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
            crosshair_at_mousepointer(ren);
            --fade_crosshair;
        }
        
        
        SDL_RenderPresent(ren);
                                                                                                                                                                                                                                                           
        f.set_current_sample(offset_x/X_AXIS_SCALE/zoom);
        //SDL_Delay(100);
    }

    Util::cleanup(ren, win);

	SDL_Quit();
	return 0;
}

