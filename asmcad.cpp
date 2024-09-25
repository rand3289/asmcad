#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <memory>
#include "object.h"
using namespace std;


shared_ptr<SDL_Texture> loadImage(string filename){
    SDL_Surface* img = SDL_LoadBMP( filename );
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
    return shared_ptr<SLD_Texture>(texture);
}


void exitSDLerr(){
    std::cerr << "SDL error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    exit(1);
}


int main(int argc, char* argv[]){
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) { exitSDLerr(); }
//    SDL_DisplayMode dm;
//    SDL_GetCurrentDisplayMode(0, &dm);

    const int SCREEN_WIDTH = 1024;
    const int SCREEN_HEIGHT = 768;
    const int WINPOS = SDL_WINDOWPOS_CENTERED;
    SDL_Window* window=SDL_CreateWindow("block bored", WINPOS, WINPOS, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(0==window){ exitSDLerr(); }
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    if(0==renderer){ exitSDLerr(); }

    shared_ptr<Object> draggedObject;
    shared_ptr<Object> root;
    Point xy;
    bool buttonDown = false;
    SDL_Event e;
    bool run = true;
    while(run){
        while( SDL_PollEvent( &e ) ){
            switch(e.type){
                case SDL_QUIT:
                    run = false;
                    break;
                case SDL_MOUSEBUTTONDOWN: // SDL_GetTicks() to get mouse click time
                    buttonDown = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    buttonDown = false;
                    Uint32 state = SDL_GetMouseState(&xy.x, &xy.y);
                    if(draggedObject){
                        root->dropped(xy, draggedObject);
                        draggedObject.reset();
                    } else if(state & SDL_BUTTON(SDL_BUTTON_LEFT)){
                        root->click(xy);
                    } else if(state & SDL_BUTTON(SDL_BUTTON_RIGHT)){
                        root->rclick(xy);
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if(!buttonDown) { break; }
                    SDL_GetMouseState(&xy.x, &xy.y);
                    if(!draggedObject){
                        draggedObject = root->takeObject(xy);
                    }
                    root->drag(xy);
                    break;
                case SDL_MOUSEWHEEL:
                    SDL_GetMouseState(&xy.x, &xy.y);
                    root->scroll(xy, event.wheel.y);
                    break;
                default:
            } // switch
        } // while

        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_ADD);

        root->draw(renderer);
        if(draggedObject){
            draggedObject->setLayout(xy);
            draggedObject->draw(renderer);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay( 16 ); // less than 60fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
