#pragma once
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <string>
#include <vector>
#include <memory>
#include "misc.h"
#include "sdltext.h"

#define ITEM_HEIGHT 150
#define ITEM_WIDTH 100

struct Object: public std::enable_shared_from_this<Object>{
    bool isClone = false;
    bool draggedOver = false;
    SDL_Rect loc; // location and dimentions of the Object
    std::shared_ptr<SDL_Texture> img; // Object's background image
    Object();

    // The way children are removed is by dragging them out but sometimes they also have to be deleted from other objects
    virtual bool removeChild(std::shared_ptr<Object>& obj){ return false; };
    virtual bool saveScad(std::ostream& file)=0; // save self and children into an openscad file
    virtual void setLocation(const Point& xy);
    virtual void setImage(std::shared_ptr<SDL_Texture> sdlTexture){ img = sdlTexture; }
    virtual void draw(SDL_Renderer* rend);
    virtual std::shared_ptr<Object> clone(){ return shared_from_this(); }; // by default just return self

    virtual std::shared_ptr<Object> click (const Point& xy){ return std::shared_ptr<Object>(); } // mouse click
    virtual std::shared_ptr<Object> clickr(const Point& xy){ return std::shared_ptr<Object>(); } // right click
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
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
    virtual std::shared_ptr<Object> takeObject(const Point& xy);
    virtual std::shared_ptr<Object> click (const Point& xy);
    virtual std::shared_ptr<Object> clickr(const Point& xy);
};

// Vertical Layout will be used in the main frame as "lines" to contain Operator and in the MODULE list
// fixed horizontal & vertical size. Changes only when main window is resized.
// scrolls on up()/down() events
// It can contain FlowLayout, Operator and Module
// when window is resized, it resizes in width and height
// TODO:    void setSize(int H, int W){ loc.h = H; loc.w = W; }
struct VerticalLayout: public FlowLayout {
    VerticalLayout(int width, int height, bool disDragDrop=false): FlowLayout(width, disDragDrop){ loc.h = height; }
    virtual void setLocation(const Point& xy);
    virtual void scroll(const Point& xy, int y);
};

struct Labels: public VerticalLayout {
    Labels(int width, int height): VerticalLayout(width, height){}
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
    std::shared_ptr<Object> takeObject(const Point& xy);
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
    Module(std::shared_ptr<Object> const & parenT): parent(parenT) { }
    virtual bool saveScad(std::ostream& file);
    virtual std::shared_ptr<Object> clone();
};


// When Operator is dropped into the "module list" it should call saveScad() on self, 
// then generate an image from saved text and then call module->setImage()
class Operator: public Object {
    std::shared_ptr<Object> module; // openscad module
    FlowLayout layout;
public:
    enum OperatorType {UNION, DIFFERENCE, INTERSECTION} type;
    Operator(OperatorType ot);
    std::shared_ptr<Object> getModule();
    virtual bool saveScad(std::ostream& file);
    virtual std::shared_ptr<Object> clone();
    virtual bool dropped(const Point& xy, std::shared_ptr<Object>const & obj);
    virtual void setLocation(const Point& xy);
    virtual void draw(SDL_Renderer* rend);
    virtual std::shared_ptr<Object> click (const Point& xy){ return layout.click(xy); }
    virtual std::shared_ptr<Object> clickr(const Point& xy){ return layout.clickr(xy); }
};

// Floating point numeric input box from which Shape and translate/rotate take their parameters
// Changes via scroll wheel up/down.  
// Change increment depends on whether Input was selected via click or right-click
class Input: public Object {  // does not have children
    double value;
    double delta;
    bool enabled;
    static std::shared_ptr<Text> printer; // writes text to screen
public:
    Input(): value(0), delta(1.0), enabled(true) {
        loc.w = 80;
        loc.h = 16;
        if(!printer){ // TODO: make printer non-pointer???
            printer = std::make_shared<Text>(16);
        }
    }
    void disable(){ enabled = false; }
    virtual void draw(SDL_Renderer* rend);
    virtual std::shared_ptr<Object> click(const Point& xy);
    virtual std::shared_ptr<Object> clickr(const Point& xy); // change it slowly after right click
    virtual void scroll(const Point& xy, int y){ value += y*delta; std::cout << 's'; std::cout.flush(); }
    virtual bool saveScad(std::ostream& file);
    void setValue(double val){ value = val; }
};

// an object with 3 input fields
class XYZ: public Object{
protected:
    std::shared_ptr<Input> x,y,z;
public:
    XYZ();
    virtual void draw(SDL_Renderer* rend);
    virtual void setLocation(const Point& xy);
    virtual std::shared_ptr<Object> click(const Point& xy);
    virtual std::shared_ptr<Object> clickr(const Point& xy);
};

// translate/rotate/scale
struct Modifier: public XYZ {
    enum ModifierType {TRANSLATE, ROTATE, SCALE} type;
    Modifier(ModifierType mt);
    virtual bool saveScad(std::ostream& file);
    virtual std::shared_ptr<Object> clone();
};

// An actual shape such as OpenScad's cube, cylinder and sphere
struct Shape: public XYZ {
    enum ShapeType {CUBE, CYLINDER, SPHERE} type;
    Shape(ShapeType st);
    virtual bool saveScad(std::ostream& file);
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
};

struct Custom: public Object {
    Custom(){ img = ImageLoader::getImage("img/custom.png"); }
    virtual bool saveScad(std::ostream& file){ return true; }
};
