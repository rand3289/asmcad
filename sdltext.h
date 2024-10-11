#ifndef INCLUDED_SDLTEXT_H
#define INCLUDED_SDLTEXT_H

#include <SDL2/SDL_ttf.h>
#include <string>
#include <iostream>


constexpr SDL_Color WHITE = { 255, 255, 255, SDL_ALPHA_OPAQUE };

// A class for printing text on the screen
// TODO: a method for returning a texture with text printed
class Text {
    TTF_Font * font = nullptr;
    const int height;
public:
    int getHeightPixels(){ return height; }

    // https://www.w3.org/TR/css3-values/#absolute-lengths
    // There are 96 pixels per inch and 72 points per inch
    Text(int points = 20): height(1+(96*points)/72) {
        if( ! TTF_WasInit() ){ TTF_Init(); }
        font = TTF_OpenFont("Roboto-Regular.ttf", points);
        if(!font) { std::cerr << "SDL2_ttf ERROR: " << TTF_GetError() << std::endl; }
    }

    virtual ~Text(){
        if(TTF_WasInit() && font) { // if TTF_CloseFont() is called after TTF_Quit(), it crashes!
            TTF_CloseFont(font);
        }
    }


// prints a string at a specified location x,y
// returns the width of the printed text in pixels.
// return value < 0, indicates an error.  User can get more information by calling SDL_GetError()
// There is a need for an error logging service to report errors
    int print(const std::string& text, int x, int y, SDL_Renderer* renderer, const SDL_Color& color = WHITE){
        if(!font) { return -1001; } // font did not load successfuly.  Is the ttf file there?
        SDL_Surface * surface = TTF_RenderText_Solid(font, text.c_str(), color);
        if(!surface) { return -1002; } // use large numbers to differentiate from SDL errors
        SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
        if(!texture){
            SDL_FreeSurface(surface);
            return -1003;
        }
        SDL_Rect rect = { x, y, surface->w, surface->h }; // now we know width and height
        int err = SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        return err ? err : rect.w; // return width of the texture which depends on the length of the text
    }
};

#endif // INCLUDED_SDLTEXT_H
