#pragma once
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <string>
#include <vector>
#include <memory>
#include "misc.h"

#define ITEM_HEIGHT 150
#define ITEM_WIDTH 100

struct Object: public std::enable_shared_from_this<Object>{
    bool isClone = false;
    bool draggedOver = false;
    SDL_Rect loc; // location of the image
    SDL_Texture* img; // this is an opaque pointer and can not be wrapped in shared_ptr. Own it
    Object();
    virtual ~Object(){ if(img){ SDL_DestroyTexture(img); } }

    // The way children are removed is by dragging them out but sometimes they also have to be deleted from other objects
    virtual bool removeChild(std::shared_ptr<Object>& obj){ return false; };
    virtual bool saveScad(std::ostream& file)=0; // save self and children into an openscad file
    virtual void setLocation(const Point& xy);
    virtual void setImage(SDL_Texture* sdlTexture){ img = sdlTexture; }
    virtual void draw(SDL_Renderer* rend);
    virtual std::shared_ptr<Object> clone(){ return shared_from_this(); }; // by default just return self

    virtual void click (const Point& xy){} // mouse click
    virtual void clickr(const Point& xy){} // right click
    virtual void scroll(const Point& xy, int y){} // mouse wheel scrolls in vertical direction

    // mouse started dragging within this object
    virtual std::shared_ptr<Object> takeObject(const Point& xy){
         return isClone ? shared_from_this() : clone(); // our system has a bizzare property where Objects can be cloned only once
    }
    // another object is dragged accross this one
    virtual void drag(const Point& xy){ draggedOver = true; }
    virtual void dragEnd(){ draggedOver = false; }
    // another object was dropped on top of this one
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj){ return false; }
};


// Left-to-right flow layout without scrolling.
// Used in top menu and to contain operators' children
class FlowLayout: public Object {
protected:
    std::vector<std::shared_ptr<Object>> children;
    bool disableDragDrop = false;
public:
    FlowLayout(int width, bool disDragDrop=false): disableDragDrop(disDragDrop) { loc.w = width; }
    void addObject(std::shared_ptr<Object>const & obj);
    virtual bool saveScad(std::ostream& file);
    virtual void setLocation(const Point& xy);
    virtual bool removeChild(std::shared_ptr<Object>& obj);
    virtual void draw(SDL_Renderer* rend);
    virtual std::shared_ptr<Object> takeObject(const Point& xy);
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
};

// Vertical Layout will be used in the main frame as "lines" to contain Operator and in the MODULE list
// fixed horizontal & vertical size. Changes only when main window is resized.
// scrolls on up()/down() events
// It can contain FlowLayout, Operator and Module
// when window is resized, it resizes in width and height
struct VerticalLayout: public FlowLayout {
    VerticalLayout(int width, int height, bool disDragDrop=false): FlowLayout(width, disDragDrop){ loc.h = height; }
    virtual void setLocation(const Point& xy);
    virtual void scroll(const Point& xy, int y);
//    void setSize(int H, int W){ loc.h = H; loc.w = W; }
};


struct Labels: public VerticalLayout {
    Labels(int width, int height): VerticalLayout(width, height){}
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
};

struct Main: public VerticalLayout {
    Main(int width, int height): VerticalLayout(width, height){}
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
};

// Graphical representation (picture) of an Operator and all its children
// This is an equivalent of OpenScad's module
class Module: public Object { // does not have children
    std::weak_ptr<Object> parent;
public:
    Module(std::shared_ptr<Object> const & parenT): parent(parenT) { isClone = true; }
    virtual bool saveScad(std::ostream& file);
};


// When Operator is dropped into the "module list" it should call saveScad() on self, 
// then generate an image from saved text and then call module->setImage()
class Operator: public Object {
    std::shared_ptr<Object> module; // openscad module
    FlowLayout layout;
public:
    enum OperatorType {UNION, DIFFERENCE, INTERSECTION} type;
    Operator(OperatorType ot);
    virtual bool saveScad(std::ostream& file);
    virtual std::shared_ptr<Object> clone();
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
    virtual void setLocation(const Point& xy);
    virtual void draw(SDL_Renderer* rend);
    std::shared_ptr<Object> getModule();
};

// Floating point numeric input box from which Shape and translate/rotate take their parameters
// Changes via scroll wheel up/down.  
// Change increment depends on whether Input was selected via click or right-click
class Input: public Object {  // does not have children
    double value;
    double delta;
public:
    Input(): value(10), delta(1.0) {}
    virtual void draw(SDL_Renderer* rend);
    virtual void click(const Point& xy){ delta = 1.0; }
    virtual void clickr(const Point& xy){ delta = 0.01; } // change it slowly after right click
    virtual bool saveScad(std::ostream& file);
};

// translate/rotate
// can "contain" Shape, Module and another Modifier
class Modifier: public Object {
    Input x,y,z;
public:
    enum ModifierType {TRANSLATE, ROTATE} type;
    Modifier(ModifierType mt);
    virtual void draw(SDL_Renderer* rend);
    virtual bool saveScad(std::ostream& file);
    virtual void setLocation(const Point& xy);
    virtual std::shared_ptr<Object> clone();
};

// These are VIEW and DELETE zones in the upper corners
// When an Operator is dropped into DELETE, it is deleted if it is unused in the rest of the "code"
// When a Module is dropped here, module representation (name) is deleted from an Operator and Module list
// When a module (operator) is dragged into VIEW, it generates and saves OpenScad code into OUTPUT_FILE_SCAD
// Code in OUTPUT_FILE_SCAD should be opened in OpenScad for real-time display
class DropZone: public Object { // does not have children
public:
    enum DZType {VIEW, DELETE} type;
    DropZone(DZType dzt): type(dzt) {
        img = ImageLoader::getImage( VIEW == type ? "img/view.png" : "img/delete.png");
    }
    virtual bool saveScad(std::ostream& file){ return true; }
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
    virtual std::shared_ptr<Object> takeObject(const Point& xy){ return std::shared_ptr<Object>(); }
};

// An actual shape such as OpenScad's cube, cylinder and sphere
class Shape: public Object { // does not have children
    Input a,b,c;
public:
    enum ShapeType {CUBE, CYLINDER, SPHERE} type;
    Shape(ShapeType st);
    virtual void draw(SDL_Renderer* rend);
    virtual bool saveScad(std::ostream& file);
    virtual std::shared_ptr<Object> clone();
};

struct Custom: public Object {
    Custom(){ img = ImageLoader::getImage("img/custom.png"); }
    virtual bool saveScad(std::ostream& file){ return true; }
};