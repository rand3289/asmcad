#pragma once
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <string>
#include <memory>

class Object;
std::shared_ptr<Object> initGui(int width, int height); // initialize layout of the application's GUI


// load SDL texture from a png image
// this class has to be initialized by calling ImageLoader::setRenderer()
class ImageLoader {
    static SDL_Renderer* renderer;
public:
    static void setRenderer(SDL_Renderer* rendereR){ renderer = rendereR; }
    static std::shared_ptr<SDL_Texture> getImage(const std::string& filename);
};


// save openscad code to a file and make an image from it
// this class has to be initialized by calling ScadSaver::setRoot()
class ScadSaver {
    static std::shared_ptr<Object> root;
public:
    static void setRoot(std::shared_ptr<Object> const & rooT){ root = rooT; }
    static bool makeObjectImage(std::shared_ptr<Object> const & obj);
};


struct Point {
    int x,y;
    Point(): x(0), y(0) {}
    Point(int X, int Y): x(X), y(Y) {}
    Point(const Point& rhs){
        x = rhs.x;
        y = rhs.y;
    }
    Point& operator=(const Point& rhs){
        x = rhs.x;
        y = rhs.y;
        return *this;
    }
    bool inRectangle(const SDL_Rect& loc) const {
        return loc.x<=x && loc.y<=y && x<=loc.x+loc.w && y<=loc.y+loc.h;
    }
};
