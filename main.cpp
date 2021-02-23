#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
#define HIGHLIGHT_INTERPOLANTS false
//Change these to alter how many numbers are marked on the axis
#define X_AXIS_NUMBER_INTERVAL 25
#define Y_AXIS_NUMBER_INTERVAL 25


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

    TTF_Init();
            
            
        TTF_Font* Sans = TTF_OpenFont("Hack-Regular.ttf", 24); //this opens a font style and sets a size
        if(!Sans) {
            printf("TTF_OpenFont: %s\n", TTF_GetError());
            // handle error
        }


        SDL_Color Black = {0, 0, 0};  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color

        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "Hellos", Black); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
        if(!Sans) {
            printf("TTF_RenderText_Solid: %s\n", TTF_GetError());
            // handle error
        }
        
        SDL_Texture* Message = SDL_CreateTextureFromSurface(ren, surfaceMessage); //now you can convert it into a texture
        if (!Message)
            std::cout << "pain\n";

        SDL_Rect Message_rect; //create a rect
        Message_rect.x = 100;  //controls the rect's x coordinate 
        Message_rect.y = 100; // controls the rect's y coordinte
        Message_rect.w = 100; // controls the width of the rect
        Message_rect.h = 100; // controls the height of the rect

        //Mind you that (0,0) is on the top left of the window/screen, think a rect as the text's box, that way it would be very simple to understand
    
    
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
        
        
        //Create axis
        SDL_SetRenderDrawColor(ren, 128, 128, 128, SDL_ALPHA_OPAQUE);
        
        //y axis
        if (offset_x <= 0 && offset_x >= -WIDTH) {
            SDL_RenderDrawLine(ren, -offset_x, 0, -offset_x, HEIGHT-1);
            int correction {static_cast<int>(offset_y)%Y_AXIS_NUMBER_INTERVAL};
            std::cout << correction << '\n';
            for (int y {correction}; y < HEIGHT; y+=Y_AXIS_NUMBER_INTERVAL) {
                SDL_RenderDrawLine(ren, -offset_x-5, y, -offset_x+5, y);
                //add text
            }
        }
        
        //x axis
        if (offset_y >= 0 && offset_y <= HEIGHT) {
            SDL_RenderDrawLine(ren, 0, offset_y, WIDTH, offset_y);
            int correction {static_cast<int>(-offset_x)%X_AXIS_NUMBER_INTERVAL};
            //std::cout << correction << '\n';
            for (int x {correction}; x < WIDTH; x+=X_AXIS_NUMBER_INTERVAL) {
                SDL_RenderDrawLine(ren, x, offset_y-5, x, offset_y+5);
                //add text
            }
        }
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
        bool black {true};
        
        double prev {f.next_sample()*Y_AXIS_SCALE*zoom};
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
        
        SDL_RenderCopy(ren, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture
        
        SDL_RenderPresent(ren);
                                                                                                                                                                                                                                                           
        f.set_current_sample(offset_x/X_AXIS_SCALE/zoom);
        //SDL_Delay(100);

        //Now since it's a texture, you have to put RenderCopy in your game loop area, the area where the whole code executes

        

        //Don't forget to free your surface and texture
        //SDL_FreeSurface(surfaceMessage);
        //SDL_DestroyTexture(Message);

    }

    Util::cleanup(ren, win);

    SDL_Quit();
    return 0;
}

