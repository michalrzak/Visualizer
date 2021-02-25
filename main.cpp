#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cmath>
#include <vector>
#include <iomanip>

#include "cleanup.h"
#include "Function.h"

//Change these numbers to alter the window size
#define WIDTH  640
#define HEIGHT  480

//Change these numbers to change the scrollspeed and direction
#define X_SCROLLSPEED 2.0
#define Y_SCROLLSPEED -2.0

//Change these number to change the zoom parameters
#define ZOOMSPEED 1.25 //this number should be kept above 1, otherwise zooming will be reversed
#define MIN_ZOOM 0.1
#define MAX_ZOOM 30
#define CROSSHAIR_FADE_DURATION 20u

//Change these to alter the graph
#define SAMPLE_FREQUENCY 0.001
#define X_AXIS_SCALE 75
#define Y_AXIS_SCALE 75
#define HIGHLIGHT_INTERPOLANTS true
//Change these to alter how many numbers are marked on the axis
#define X_AXIS_NUMBER_INTERVAL 50
#define Y_AXIS_NUMBER_INTERVAL 50
//Whenever a marked interval is larger then this it will be split in two
//Same way, when an interval becomes less then half of this it will be merged
#define AXIS_MAX_INTERVAL_SIZE 75
//Font size on numbers on axis
#define AXIS_FONT_SIZE 10


std::ostream& logSDLError(std::ostream &os, const std::string &msg){
    return os << msg << " error: " << SDL_GetError() << '\n';
}
std::ostream& logTTFError(std::ostream &os, const std::string &msg){
    return os << msg << " error: " << TTF_GetError() << '\n';
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

    
    //Font initialization
    TTF_Init();
    
    TTF_Font* font = TTF_OpenFont("Hack-Regular.ttf", AXIS_FONT_SIZE);
    if(!font) {
        logTTFError(std::cerr, "OpenFont");
        Util::cleanup(ren, win);
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
    int offset_x {0};
    int offset_y {0};
    
    bool done {false};
    while (!done) {
        
        SDL_Event event;

        SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(ren);
        
        
        //Create axis
        SDL_SetRenderDrawColor(ren, 128, 128, 128, SDL_ALPHA_OPAQUE);
        
        std::vector<SDL_Surface*> message_surfaces;
        std::vector<SDL_Texture*> messages;
        
        
        double log {std::log2(zoom)}; //maybe change the log2 here to some other base to alter when a split occurs
        double divide {std::pow(2, std::floor(log))};
        
        //y axis
        if (offset_x <= 0 && offset_x >= -WIDTH) {
            SDL_RenderDrawLine(ren, -offset_x, 0, -offset_x, HEIGHT-1);
            
            double y_num {std::ceil(static_cast<double>(-offset_y)/Y_AXIS_SCALE*divide/zoom)};
            
            int step {static_cast<int>(Y_AXIS_SCALE*zoom/divide)};
            int correction {offset_y%step};
            
            
            for (int y {correction}; y < HEIGHT; y+=step, ++y_num) {
                
                SDL_RenderDrawLine(ren, -offset_x-5, y, -offset_x+5, y);
                
                //std::string num {std::to_string((offset_y-y)/Y_AXIS_SCALE/zoom)};
                std::string num {std::to_string(-y_num/divide)};
                num.resize(num.find('.') + 3); //+1 for the dot it self + 2 for two decimal places
                
                message_surfaces.push_back(TTF_RenderText_Solid(font, num.c_str(), {200, 125, 0}));
                if(!message_surfaces.back()) {
                    logTTFError(std::cerr, "TTF_RenderText_Solid");
                    
                    message_surfaces.pop_back(); //remove the last, invalid element
                    for (auto ele : message_surfaces)
                        Util::cleanup(ele);
                    Util::cleanup(ren, win);
                    
                    SDL_Quit();
                    return 1;
                }
                
                messages.push_back(SDL_CreateTextureFromSurface(ren, message_surfaces.back()));
                if (!messages.back()) {
                    logTTFError(std::cerr, "TTF_RenderText_Solid");
                    
                    messages.pop_back(); //remove the last, invalid element
                    for (auto ele : messages)
                        Util::cleanup(ele);
                    
                    for (auto ele : message_surfaces)
                        Util::cleanup(ele);
                    
                    Util::cleanup(ren, win);
                    
                    SDL_Quit();
                    return 1;
                }

                SDL_Rect message_rect;
                message_rect.x = -offset_x+6;
                message_rect.y = y;
                message_rect.w = num.size()*AXIS_FONT_SIZE;
                message_rect.h = AXIS_FONT_SIZE*1.5;
                
                SDL_RenderCopy(ren, messages.back(), NULL, &message_rect);
            }
        }
        
        //x axis
        if (offset_y >= 0 && offset_y <= HEIGHT) {
            
            SDL_RenderDrawLine(ren, 0, offset_y, WIDTH, offset_y);
            
            double x_num {std::ceil(static_cast<double>(offset_x)/X_AXIS_SCALE*divide/zoom)};
            
            int step {static_cast<int>(X_AXIS_SCALE*zoom/divide)};
            int correction {-offset_x%step}; //correction
            
            //std::cout << x_num << ' ' << offset_x/zoom/X_AXIS_SCALE*divide << ' ' << divide << ' ' << correction << '\n';
            
            
            for (int x {correction}; x < WIDTH; x+=step, ++x_num) {
                SDL_RenderDrawLine(ren, x, offset_y-5, x, offset_y+5);
                
                //dont show 0 on the x axis
                if (std::abs((x+offset_x)/X_AXIS_SCALE/zoom) < 0.01)
                    continue;

                
                std::string num {std::to_string(x_num/divide)};
                num.resize(num.find('.') + 3); //+1 for the dot it self + 2 for two decimal places
                
                
                message_surfaces.push_back(TTF_RenderText_Solid(font, num.c_str(), {200, 125, 0}));
                if(!message_surfaces.back()) {
                    logTTFError(std::cerr, "TTF_RenderText_Solid");
                    
                    message_surfaces.pop_back(); //remove the last, invalid element
                    for (auto ele : message_surfaces)
                        Util::cleanup(ele);
                    Util::cleanup(ren, win);
                    
                    SDL_Quit();
                    return 1;
                }
                
                messages.push_back(SDL_CreateTextureFromSurface(ren, message_surfaces.back()));
                if (!messages.back()) {
                    logTTFError(std::cerr, "TTF_RenderText_Solid");
                    
                    messages.pop_back(); //remove the last, invalid element
                    for (auto ele : messages)
                        Util::cleanup(ele);
                    
                    for (auto ele : message_surfaces)
                        Util::cleanup(ele);
                    
                    Util::cleanup(ren, win);
                    
                    SDL_Quit();
                    return 1;
                }

                SDL_Rect message_rect;
                message_rect.x = x-num.size()*AXIS_FONT_SIZE/2;
                message_rect.y = offset_y+6;
                message_rect.w = num.size()*AXIS_FONT_SIZE;
                message_rect.h = AXIS_FONT_SIZE*1.5;
                
                SDL_RenderCopy(ren, messages.back(), NULL, &message_rect);
            }
        }
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
        bool black {true};
        
        double prev {f.next_sample()*Y_AXIS_SCALE*zoom};
        for (double x {0}; x < WIDTH; x+=SAMPLE_FREQUENCY*X_AXIS_SCALE*zoom) {
            double next = f.next_sample()*Y_AXIS_SCALE*zoom;
            
            //Check if the drawn line would be visible on screen
            if ( ((offset_y - prev) >= 0 && (offset_y-prev) <= HEIGHT) || ( (offset_y - next) >= 0 && (offset_y - next) <= HEIGHT) ) {
                SDL_RenderDrawLine(ren, x, offset_y - prev , x+SAMPLE_FREQUENCY*X_AXIS_SCALE*zoom, offset_y - next);
                
                if (HIGHLIGHT_INTERPOLANTS) {
                    if (black)
                        SDL_SetRenderDrawColor(ren, 255, 0, 0, SDL_ALPHA_OPAQUE);
                    else
                        SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    black = !black;
                }
            }
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
                                                                                                                                                                                                                                                           
        f.set_current_sample(static_cast<double>(offset_x)/X_AXIS_SCALE/zoom);
        
        for (auto ele : messages)
            Util::cleanup(ele);
        
        for (auto ele : message_surfaces)
            Util::cleanup(ele);

    }

    Util::cleanup(ren, win);

    SDL_Quit();
    return 0;
}

