#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <SDL2/SDL_image.h> // for loading PNG images
#include <memory>
#include <iostream>
#include "object.h"
using namespace std;


void exitSDLerr(){
    std::cerr << "SDL error: " << SDL_GetError() << std::endl;
    IMG_Quit();
    SDL_Quit();
    exit(1);
}


int main(int argc, char* argv[]){
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) { exitSDLerr(); } // Initialize SDL2 library
    if( !( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) ) { exitSDLerr(); } // Initialize PNG loading
//    SDL_DisplayMode dm;
//    SDL_GetCurrentDisplayMode(0, &dm);

    const int SCREEN_WIDTH = 1200;
    const int SCREEN_HEIGHT = 900;
    const int WINPOS = SDL_WINDOWPOS_CENTERED;
    SDL_Window* window=SDL_CreateWindow("ASM CAD", WINPOS, WINPOS, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(0==window){ exitSDLerr(); }
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    if(0==renderer){ exitSDLerr(); }
    ImageLoader::setRenderer(renderer);

    shared_ptr<Object> root = initGui(SCREEN_WIDTH, SCREEN_HEIGHT);
    shared_ptr<Object> draggedObject; // if not null, mouse is dragging this object
    shared_ptr<Object> inFocus; // when an object is clicked on, it becomes in focus and receives mouse wheel events
    Point xy;
    bool buttonDown = false;
    SDL_Event e;
    bool run = true;

    while(run){
        while( SDL_PollEvent( &e ) ){
            SDL_GetMouseState(&xy.x, &xy.y);
            switch(e.type){
                case SDL_QUIT:
                    run = false;
                    break;
                case SDL_MOUSEBUTTONDOWN: // SDL_GetTicks() to get mouse click time
                    buttonDown = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    buttonDown = false;
                    if(draggedObject){
                        root->dropped(xy, draggedObject);
                        draggedObject.reset();
                    } else if(e.button.button == SDL_BUTTON_LEFT){
                        inFocus = root->click(xy);
                    } else if(e.button.button == SDL_BUTTON_RIGHT){
                        inFocus = root->clickr(xy);
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if(!buttonDown) { break; }
                    if(!draggedObject){
                        draggedObject = root->takeObject(xy);
                        if(!draggedObject){     // if this object can not be dragged
                            buttonDown = false; // end drag
                        }
                    }
                    root->drag(xy);
                    break;
                case SDL_MOUSEWHEEL:
                    if(!inFocus){ break; }
                    inFocus->scroll(xy, e.wheel.y);
                    break;
                default: break;
            } // switch
        } // while

        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_ADD);

        root->draw(renderer);
        if(draggedObject){
            draggedObject->setLocation(xy);
            draggedObject->draw(renderer);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay( 16 ); // less than 60fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
