#pragma once
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <string>
#include <memory>

class Object;
bool makeObjectImage(std::shared_ptr<Object>& obj); // makes an image of an Operator and set its corresponding Label
std::shared_ptr<Object> initGui(int width, int height); // initialize layout of the application's GUI


// load SDL texture from a png image
// this class has to be initialized by calling ImageLoader::setRenderer()
class ImageLoader {
    static SDL_Renderer* renderer;
public:
    static void setRenderer(SDL_Renderer* rendereR){ renderer = rendereR; }
    static SDL_Texture* getImage(const std::string& filename);
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
//    bool inRectangle(const Point ul, const Point lr);
    bool inRectangle(const SDL_Rect& loc) const { return loc.x<=x && loc.y<=x && x<=loc.x+loc.w && y<=loc.y+loc.h; }
    Point operator+(const Point& rhs){ return Point(x+rhs.x,y+rhs.y); }
};
