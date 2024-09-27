#pragma once
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <string>
#include <vector>
#include <memory>


struct Point {
    int x;
    int y;
    Point(): x(0), y(0) {}
    Point(int X, int Y): x(X), y(Y) {}
    Point& operator+(const Point& rhs);
    bool inRectangle(Point ul, Point lr);
};


struct Object {
    std::shared_ptr<SDL_Texture> img;
    SDL_Rect loc; // location of the image

    // The way children are removed is by dragging them out but sometimes they also have to be deleted from other objects
    virtual bool removeChild(std::shared_ptr<Object&> obj);
    virtual bool saveScad(std::ofstream& file); // save self and children into an openscad file
    virtual void setLayout(const Point& xy);
    virtual void setImage(std::shared_ptr<SDL_Texture>& sdlTexture){ img = sdlTexture; }
    virtual void draw(SDL_Renderer* rend){ SDL_RenderCopy(rend, img.get(), NULL, &loc); }
    virtual std::shared_ptr<Object> clone();

    virtual void click (const Point& xy); // mouse click
    virtual void clickr(const Point& xy); // right click
    virtual void scroll(const Point& xy, int y); // mouse wheel scrolls in vertical direction

    virtual std::shared_ptr<Object> takeObject(const Point& xy); // mouse started dragging within this object
    virtual void drag(const Point& xy); // another object is dragged accross this one
    virtual void dragEnd();
    virtual void dropped(const Point& xy, std::shared_ptr<Object>& obj); // another object was dropped on top of this one
};

// Should "delete drop zone" be a root?
// Use shared_ptr value converted to int as a unique ID when saving code?

// 2 layouts Vertical and Flow.  Vertical will be used in the main frame as "lines" and in the MODULE list
// Flow layout will be used in top menu and to contain operators within "lines"

// fixed horizontal & vertical size
// scrolls on up()/down() events
class VerticalLayout: public Object {
    std::vector<Object> children;
};
// no scrolling left-to-right layout
class FlowLayout: public Object {
    std::vector<Object> children;
};

// Operator's takeObject() and dropped() call saveScad() on self, 
// then generate an image from saved text and then call module->setImage()

class Operator: public Object {
    std::shared_ptr<Object> module; // openscad module
    FlowLayout layout;
};
class DropZone: public Object { // does not have children

};
class Input: public Object {  // does not have children // simple input box
    double value;
};
class Shape: public Object { // does not have children
    enum ShapeType {UNION, DIFFERENCE, INTERSECTION} type;
};
class Module: public Object { // does not have children

};


struct ImageLoader {
    static SDL_Renderer* renderer;
    static void setRenderer(SDL_Renderer* rendereR){ renderer = rendereR; }
    static bool setObjectImage(std::shared_ptr<Object>& obj, const std::string& filename);
};
