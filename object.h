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


struct Object: public std::enable_shared_from_this<Object>{
    SDL_Rect loc; // location of the image
    SDL_Texture* img; // this is an opaque pointer and can not be wrapped in shared_ptr. Own it
    Object(): img(nullptr) {}
    Object(const std::string imgFileName);
    virtual ~Object(){ if(img){ SDL_DestroyTexture(img); } }

    // The way children are removed is by dragging them out but sometimes they also have to be deleted from other objects
    virtual bool removeChild(std::shared_ptr<Object> obj){ return true; };
    virtual bool saveScad(std::ofstream& file)=0; // save self and children into an openscad file
    virtual void setLayout(const Point& xy){}
    virtual void setImage(SDL_Texture* sdlTexture){ img = sdlTexture; }
    virtual void draw(SDL_Renderer* rend){ SDL_RenderCopy(rend, img, NULL, &loc); }
//    virtual std::shared_ptr<Object> clone()=0;

    virtual void click (const Point& xy){} // mouse click
    virtual void clickr(const Point& xy){} // right click
    virtual void scroll(const Point& xy, int y){} // mouse wheel scrolls in vertical direction

    virtual std::shared_ptr<Object> takeObject(const Point& xy){ return shared_from_this(); } // mouse started dragging within this object
    virtual void drag(const Point& xy){} // another object is dragged accross this one
    virtual void dragEnd(){}
    virtual void dropped(const Point& xy, std::shared_ptr<Object>& obj){} // another object was dropped on top of this one
};

// Should "delete drop zone" be a root?
// Use shared_ptr value converted to int as a unique ID when saving code?

// 2 layouts Vertical and Flow.  Vertical will be used in the main frame as "lines" and in the MODULE list
// Flow layout will be used in top menu and to contain operators within "lines"

// fixed horizontal & vertical size
// scrolls on up()/down() events
class VerticalLayout: public Object {
    std::vector<Object> children;
public:
    virtual bool removeChild(std::shared_ptr<Object&> obj);
    virtual bool saveScad(std::ofstream& file);
};

// no scrolling left-to-right layout
class FlowLayout: public Object {
    std::vector<Object> children;
public:
    virtual bool removeChild(std::shared_ptr<Object&> obj);
    virtual bool saveScad(std::ofstream& file);
};

// Operator's takeObject() and dropped() call saveScad() on self, 
// then generate an image from saved text and then call module->setImage()
class Operator: public Object {
    std::shared_ptr<Object> module; // openscad module
    FlowLayout layout;
public:
    virtual bool saveScad(std::ofstream& file);
};

class DropZone: public Object { // does not have children
public:
    virtual bool saveScad(std::ofstream& file);
};

class Input: public Object {  // does not have children // simple input box
    double value;
public:
    virtual bool saveScad(std::ofstream& file);
};

class Shape: public Object { // does not have children
    enum ShapeType {UNION, DIFFERENCE, INTERSECTION} type;
public:
    virtual bool saveScad(std::ofstream& file);
};

class Module: public Object { // does not have children
public:
    virtual bool saveScad(std::ofstream& file);
};


// load SDL texture from a png image
// this class has to be initialized by calling ImageLoader::setRenderer()
class ImageLoader {
    static SDL_Renderer* renderer;
public:
    static void setRenderer(SDL_Renderer* rendereR){ renderer = rendereR; }
    static SDL_Texture* getImage(const std::string& filename);
};
